#include "Cube222.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <algorithm>

using namespace std;

// Multi-threaded DFS solver class
class Cube222MT : public Cube222 {
public:
	Cube222MT(Color initialColor = Color::WHITE, int cRow = 2, int cCol = 2, int cFace = 6)
		: Cube222(initialColor, cRow, cCol, cFace) {
	}

	Cube* copy() const override {
		Cube222MT* newCube = new Cube222MT(*this);
		newCube->_matrix = this->_matrix;
		return newCube;
	}

	static bool isRedundantMove(Rotation lastMove, Rotation currentMove) {
		// Don't allow inverse immediately after a move
		auto it = inverseRotation.find(lastMove);
		if (it != inverseRotation.end() && it->second == currentMove) return true;
		// Don't allow same move twice in a row
		if (lastMove == currentMove) return true;
		return false;
	}

	// Multi-threaded DFS solver
	void dfsParallel(int depth = 1, const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		if (isSolved()) {
			return;
		}

		static const std::vector<Rotation> allRotations = { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };

		// Shared state for threads
		std::atomic<bool> solutionFound{ false };
		std::mutex solutionMutex;
		std::vector<Rotation> foundSolution;

		// Get hardware thread count
		unsigned int numThreads = std::thread::hardware_concurrency();
		if (numThreads == 0) numThreads = 4; // Fallback

		std::cout << "Using " << numThreads << " threads for parallel search.\n";

		// Generate all combinations
		std::vector<Rotation> currentPath;
		std::vector<std::vector<Rotation>> potentialSolutions;
		generateCombinations(allRotations, depth, currentPath, potentialSolutions);

		std::cout << potentialSolutions.size() << " combinations to test at depth " << depth << ".\n";

		// Partition work across threads
		size_t totalWork = potentialSolutions.size();
		size_t workPerThread = (totalWork + numThreads - 1) / numThreads;

		std::vector<std::thread> threads;
		std::atomic<size_t> testedCount{ 0 };

		auto worker = [&](size_t startIdx, size_t endIdx) {
			// Each thread gets its own cube copy
			Cube222MT* localCube = static_cast<Cube222MT*>(this->copy());
			localCube->_matrix = this->_initMatrix;

			for (size_t i = startIdx; i < endIdx && !solutionFound.load(); ++i) {
				localCube->reset();
				const auto& solution = potentialSolutions[i];

				localCube->applySolution(solution);

				if (localCube->isSolved()) {
					solutionFound.store(true);

					std::lock_guard<std::mutex> lock(solutionMutex);
					foundSolution = solution;
					break;
				}

				testedCount.fetch_add(1);

				// Progress reporting every 10000 tests
				if (testedCount.load() % 100000 == 0) {
					std::cout << "Tested: " << testedCount.load() << " / " << totalWork << "\n";
				}
			}

			delete localCube;
		};

		// Launch threads
		auto startTime = std::chrono::steady_clock::now();

		for (unsigned int t = 0; t < numThreads; ++t) {
			size_t startIdx = t * workPerThread;
			size_t endIdx = std::min(startIdx + workPerThread, totalWork);

			if (startIdx < totalWork) {
				threads.emplace_back(worker, startIdx, endIdx);
			}
		}

		// Wait for all threads
		for (auto& thread : threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}

		auto endTime = std::chrono::steady_clock::now();
		std::chrono::duration<double> timeTaken = endTime - begin_time;

		if (solutionFound.load()) {
			std::cout << "Solved in " << timeTaken.count() << " seconds.\n";
			std::cout << "Tested " << testedCount.load() << " combinations.\n";
			std::cout << "Solution: ";
			for (Rotation move : foundSolution) {
				std::cout << rotationToString(move) << " ";
			}
			std::cout << "\n";

			// Apply solution to main cube
			reset();
			applySolution(foundSolution);
			return;
		}

		std::cout << timeTaken.count() << " seconds elapsed.\n";
		std::cout << "Increasing depth to " << depth + 1 << ". Continue search...\n";
		dfsParallel(depth + 1, begin_time);
	}

private:
	void generateCombinations(const std::vector<Rotation>& allRotations, int depth, std::vector<Rotation>& currentPath, std::vector<std::vector<Rotation>>& results) {
		if (depth == 0) {
			results.push_back(currentPath);
			return;
		}

		for (Rotation r : allRotations) {
			currentPath.push_back(r);
			generateCombinations(allRotations, depth - 1, currentPath, results);
			currentPath.pop_back();
		}
	}
};

int main(int argc, char* argv[]) {
	Cube222MT cube;

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

	std::cout << "2x2x2 Cube (Multi-Threaded DFS Solver):" << std::endl;
	cube.printState();

	auto startTime = std::chrono::steady_clock::now();
	cube.dfsParallel(1, startTime);

	cube.printState();

	return 0;
}
