#include "Cube222.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <climits>
#include <chrono>
#include <iostream>
#include <algorithm>

using namespace std;

// IDA* Multi-threaded solver class
class Cube222OptMT : public Cube222 {
public:
	static const int FOUND = -1;
	size_t _nodesExplored = 0;

	Cube222OptMT(Color initialColor = Color::WHITE, int cRow = 2, int cCol = 2, int cFace = 6)
		: Cube222(initialColor, cRow, cCol, cFace) {
	}

	Cube* copy() const override {
		Cube222OptMT* newCube = new Cube222OptMT(*this);
		newCube->_matrix = this->_matrix;
		return newCube;
	}

	// Heuristic function: counts pieces not matching target solved state
	int heuristic() const {
		int misplaced = 0;
		// Define the standard solved state colors for each face
		const Color solvedColors[6] = { YELLOW, BLUE, RED, WHITE, GREEN, ORANGE };
		// Order is: TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT

		// Count misplaced pieces on first 3 faces
		for (size_t f = 0; f < _cFace / 2; ++f) {
			const Color targetColor = solvedColors[f];
			const auto& face = _matrix[f];
			for (size_t i = 0; i < _cCol; ++i) {
				for (size_t j = 0; j < _cRow; ++j) {
					if (face[i][j] != targetColor) {
						misplaced++;
					}
				}
			}
		}
		// Divide by 4 for 2x2x2 (more optimistic = more admissible)
		return (misplaced + 3) / 4;  // Round up division
	}

	static bool isRedundantMove(Rotation lastMove, Rotation currentMove) {
		if (lastMove == ROTATION_NONE) return false;
		auto it = inverseRotation.find(lastMove);
		if (it != inverseRotation.end() && it->second == currentMove) return true;
		if (lastMove == currentMove) return true;
		return false;
	}

	// Multi-threaded IDA* with heuristic
	void idaStarMT(const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		if (isSolved()) {
			std::cout << "Already solved!\n";
			return;
		}

		unsigned int numThreads = std::thread::hardware_concurrency();
		if (numThreads == 0) numThreads = 4;

		std::cout << "Using " << numThreads << " threads for parallel IDA* search.\n";

		// Shared state
		std::atomic<bool> solutionFound{ false };
		std::mutex solutionMutex;
		std::vector<Rotation> foundSolution;
		std::atomic<size_t> totalNodesExplored{ 0 };

		// Start with heuristic as initial bound
		int bound = heuristic();
		std::cout << "Initial heuristic: " << bound << " moves\n";

		while (!solutionFound.load() && bound <= 20) {
			std::cout << "Searching with bound " << bound << "...\n";

			// Partition first-level moves across threads
			static const std::vector<Rotation> allRotations = { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };

			std::vector<std::thread> threads;
			std::atomic<int> nextBoundAtomic{ INT_MAX };

			auto worker = [&](Rotation firstMove) {
				if (solutionFound.load()) return;

				// Each thread gets its own cube copy
				Cube222OptMT* localCube = static_cast<Cube222OptMT*>(this->copy());
				localCube->_matrix = this->_initMatrix;

				// Apply first move
				localCube->applyRotation(firstMove);
				std::vector<Rotation> path = { firstMove };

				// Search from this starting point
				int localNextBound = localCube->idaStarRecursiveHelper(
					1, bound, firstMove, path, solutionFound, begin_time);

				if (localNextBound == FOUND) {
					std::lock_guard<std::mutex> lock(solutionMutex);
					if (foundSolution.empty()) {
						// Store the solution path
						foundSolution = path;
					}
				}

				// Update next bound
				int currentNextBound = nextBoundAtomic.load();
				while (localNextBound < currentNextBound) {
					if (nextBoundAtomic.compare_exchange_weak(currentNextBound, localNextBound)) {
						break;
					}
				}

				totalNodesExplored.fetch_add(localCube->_nodesExplored);
				delete localCube;
			};

			// Launch one thread per first move
			for (Rotation firstMove : allRotations) {
				threads.emplace_back(worker, firstMove);
			}

			// Wait for all threads
			for (auto& thread : threads) {
				if (thread.joinable()) {
					thread.join();
				}
			}

			if (solutionFound.load()) {
				auto endTime = std::chrono::steady_clock::now();
				std::chrono::duration<double> timeTaken = endTime - begin_time;
				std::cout << "Solved in " << timeTaken.count() << " seconds.\n";
				std::cout << "Nodes explored: " << totalNodesExplored.load() << "\n";
				std::cout << "Solution (" << foundSolution.size() << " moves): ";
				for (Rotation move : foundSolution) {
					std::cout << rotationToString(move) << " ";
				}
				std::cout << "\n";
				reset();
				applySolution(foundSolution);
				return;
			}

			int nextBound = nextBoundAtomic.load();
			if (nextBound == INT_MAX) {
				std::cout << "No solution found.\n";
				return;
			}

			bound = nextBound;
		}
		std::cout << "No solution found within bound limit.\n";
	}

	// Helper for recursive search (used by threads)
	int idaStarRecursiveHelper(int currentDepth, int bound, Rotation lastMove,
		std::vector<Rotation>& path,
		std::atomic<bool>& solutionFound,
		const std::chrono::time_point<std::chrono::steady_clock>& begin_time) {

		_nodesExplored++;

		// Check if another thread found solution
		if (solutionFound.load()) {
			return FOUND;
		}

		// Calculate f-cost = g (current depth) + h (heuristic)
		int f_cost = currentDepth + heuristic();

		// Prune if f-cost exceeds bound
		if (f_cost > bound) {
			return f_cost;
		}

		// Solution found
		if (isSolved()) {
			solutionFound.store(true);
			return FOUND;
		}

		static const std::vector<Rotation> allRotations = { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };

		int minBound = INT_MAX;

		for (Rotation r : allRotations) {
			if (solutionFound.load()) {
				return FOUND;
			}

			// Prune redundant moves
			if (isRedundantMove(lastMove, r)) {
				continue;
			}

			// Apply move
			applyRotation(r);
			path.push_back(r);

			// Recurse
			int nextBound = idaStarRecursiveHelper(currentDepth + 1, bound, r, path, solutionFound, begin_time);

			// Backtrack
			undoRotation(r);
			path.pop_back();

			if (nextBound == FOUND) {
				return FOUND;
			}

			if (nextBound < minBound) {
				minBound = nextBound;
			}
		}

		return minBound;
	}
};

int main(int argc, char* argv[]) {
	Cube222OptMT cube;

	for (int i = 1; i < argc; i += 2) {
		if (i + 1 < argc) {
			std::string tag = argv[i];
			std::string values = argv[i + 1];
			std::vector<Color> colors;

			std::transform(values.begin(), values.end(), std::back_inserter(colors),
				[](char c) -> Color { return charToColor.count(c) > 0 ? charToColor[c] : UNDEFINED; });

			if (tagToFace.count(tag) > 0) {
				cube.setColor(tagToFace[tag], colors);
			}
			else {
				std::cout << "Invalid face tag: " << tag << std::endl;
			}
		}
	}

	cube.saveInitState();

	std::cout << "2x2x2 Cube (Multi-Threaded IDA* with Heuristic):" << std::endl;
	cube.printState();

	auto startTime = std::chrono::steady_clock::now();
	cube.idaStarMT(startTime);

	cube.printState();

	return 0;
}
