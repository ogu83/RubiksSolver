#include "Cube222.h"
#include <climits>
#include <chrono>
#include <iostream>
#include <algorithm>

using namespace std;

// Single-threaded IDA* solver class
class Cube222Opt : public Cube222 {
public:
	static constexpr int FOUND = -1;
	bool _solutionFound = false;
	std::vector<Rotation> _solution;
	size_t _nodesExplored = 0;

	Cube222Opt(Color initialColor = Color::WHITE, int cRow = 2, int cCol = 2, int cFace = 6)
		: Cube222(initialColor, cRow, cCol, cFace) {
	}

	Cube* copy() const override {
		Cube222Opt* newCube = new Cube222Opt(*this);
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

	// IDA* search with heuristic integration
	void idaStar(const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		if (isSolved()) {
			std::cout << "Already solved!\n";
			return;
		}

		_solutionFound = false;
		_solution.clear();
		_nodesExplored = 0;

		// Start with heuristic as initial bound
		int bound = heuristic();
		std::cout << "Initial heuristic: " << bound << " moves\n";

		while (!_solutionFound && bound <= 20) {
			std::cout << "Searching with bound " << bound << "...\n";

			std::vector<Rotation> path;
			int nextBound = idaStarRecursive(0, bound, ROTATION_NONE, path, begin_time);

			if (_solutionFound) {
				auto endTime = std::chrono::steady_clock::now();
				std::chrono::duration<double> timeTaken = endTime - begin_time;
				std::cout << "Solved in " << timeTaken.count() << " seconds.\n";
				std::cout << "Nodes explored: " << _nodesExplored << "\n";
				std::cout << "Solution (" << _solution.size() << " moves): ";
				for (Rotation move : _solution) {
					std::cout << rotationToString(move) << " ";
				}
				std::cout << "\n";

				reset();
				applySolution(_solution);
				return;
			}

			if (nextBound == INT_MAX) {
				std::cout << "No solution found.\n";
				return;
			}

			bound = nextBound;
		}
		std::cout << "No solution found within bound limit.\n";
	}

private:
	// IDA* recursive search with f-cost pruning
	int idaStarRecursive(int currentDepth, int bound, Rotation lastMove,
		std::vector<Rotation>& path,
		const std::chrono::time_point<std::chrono::steady_clock>& begin_time) {
		_nodesExplored++;

		// Calculate f-cost = g (current depth) + h (heuristic)
		int f_cost = currentDepth + heuristic();

		// Prune if f-cost exceeds bound
		if (f_cost > bound) {
			return f_cost;  // Return minimum f-cost that exceeded bound
		}

		// Solution found
		if (isSolved()) {
			std::cout << "DEBUG: isSolved() returned true at depth " << currentDepth << std::endl;
			std::cout << "DEBUG: Path: ";
			for (auto m : path) std::cout << rotationToString(m) << " ";
			std::cout << std::endl;

			// Verify the cube state
			std::cout << "DEBUG: First 3 faces - TOP[0][0]=" << (int)_matrix[TOP][0][0]
					  << " FRONT[0][0]=" << (int)_matrix[FRONT][0][0]
					  << " RIGHT[0][0]=" << (int)_matrix[RIGHT][0][0] << std::endl;

			_solutionFound = true;
			_solution = path;
			return FOUND;
		}

		static const std::vector<Rotation> allRotations = { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };

		int minBound = INT_MAX;

		for (Rotation r : allRotations) {
			// Prune redundant moves
			if (isRedundantMove(lastMove, r)) {
				continue;
			}

			// Apply move
			applyRotation(r);
			path.push_back(r);

			// Recurse
			int newBound = idaStarRecursive(currentDepth + 1, bound, r, path, begin_time);

			if (_solutionFound) {
				return FOUND;
			}

			// Track minimum bound that exceeded current bound
			if (newBound < minBound) {
				minBound = newBound;
			}

			// Undo move (backtrack)
			undoRotation(r);
			path.pop_back();
		}

		return minBound;
	}
};

int main(int argc, char* argv[]) {
	Cube222Opt cube;

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

	std::cout << "2x2x2 Cube (IDA* with Heuristic):" << std::endl;
	cube.printState();

	auto startTime = std::chrono::steady_clock::now();
	cube.idaStar(startTime);

	cube.printState();

	return 0;
}
