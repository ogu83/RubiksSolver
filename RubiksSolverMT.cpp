#include "RubiksSolver.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

using namespace std;

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE };
enum Rotation { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };

std::map<char, Color> charToColor = {
	{'R', RED}, {'B', BLUE}, {'O', ORANGE}, {'G', GREEN}, {'W', WHITE}, {'Y', YELLOW}
};

std::map<std::string, Faces> tagToFace = {
	{"-ft", TOP}, {"-ff", FRONT}, {"-fr", RIGHT}, {"-fb", BOTTOM}, {"-fbk", BACK}, {"-fl", LEFT}
};

// Inverse rotation lookup table for move pruning
const Rotation inverseRotation[] = { UI, DI, RI, LI, FI, BI, U, D, R, L, F, B };

class CubeMT {
public:
	CubeMT(Color initialColor, int cRow, int cCol, int cFace)
		: _cRow(cRow), _cCol(cCol), _cFace(cFace),
		_matrix(cFace, std::vector<std::vector<Color>>(cCol, std::vector<Color>(cRow, initialColor))) {
		setColorsToInitState();
	}

	void setColorsToInitState() {
		setColor(FRONT, BLUE);
		setColor(RIGHT, RED);
		setColor(TOP, YELLOW);
		setColor(BOTTOM, WHITE);
		setColor(BACK, GREEN);
		setColor(LEFT, ORANGE);
		_rotations.clear();
	}

	void saveInitState() {
		_initMatrix = _matrix;
	}

	void reset() {
		_matrix = _initMatrix;
		_rotations.clear();
	}

	void setColor(Faces face, const std::vector<Color>& colors) {
		for (int i = 0; i < _cRow; ++i) {
			for (int j = 0; j < _cCol; ++j) {
				int idx = i * _cCol + j;
				if (idx < colors.size()) {
					_matrix[face][i][j] = colors[idx];
				}
			}
		}
	}

	void setColor(Faces face, Color color) {
		for (int r = 0; r < _cRow; r++) {
			for (int c = 0; c < _cCol; c++) {
				setColor(face, r, c, color);
			}
		}
	}

	void setColor(Faces face, int row, int col, Color color) {
		if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
			_matrix[face][row][col] = color;
		}
	}

	Color getColor(Faces face, int row, int col) const {
		if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
			return _matrix[face][row][col];
		}
		return Color::WHITE;
	}

	virtual void applyRotation(Rotation r) {
		applyRotationInternal(r);
		_rotations.push_back(r);
	}

	void applySolution(const std::vector<Rotation>& solution) {
		for (Rotation move : solution) {
			applyRotation(move);
		}
	}

	void printCube(bool shortPrint = false) {
		std::string solvedStr = isSolved() ? "YES" : "NO";
		std::cout << "Solved: " << solvedStr << std::endl;
		std::cout << "Rotations: " << rotationsToString() << std::endl;
		if (shortPrint) {
			for (int f = 0; f < _cFace / 2; ++f) {
				std::cout << "Face: " << faceToString((Faces)f) << std::endl;
				for (const auto& row : _matrix[f]) {
					for (Color color : row) {
						std::cout << colorToString(color) << " ";
					}
					std::cout << std::endl;
				}
			}
		}
		else {
			for (int f = 0; f < _cFace; ++f) {
				std::cout << "Face: " << faceToString((Faces)f) << std::endl;
				for (const auto& row : _matrix[f]) {
					for (Color color : row) {
						std::cout << colorToString(color) << " ";
					}
					std::cout << std::endl;
				}
				std::cout << std::endl;
			}
		}
	}

	inline bool isSolved() const {
		for (size_t f = 0; f < _cFace / 2; ++f) {
			const auto& face = _matrix[f];
			const Color referenceColor = face[0][0];
			for (size_t i = 0; i < _cCol; ++i) {
				for (size_t j = 0; j < _cRow; ++j) {
					if (face[i][j] != referenceColor) {
						return false;
					}
				}
			}
		}
		return true;
	}

	static bool isRedundantMove(Rotation lastMove, Rotation currentMove) {
		// Don't allow inverse immediately after a move
		if (inverseRotation[lastMove] == currentMove) return true;
		// Don't allow same move twice in a row
		if (lastMove == currentMove) return true;
		return false;
	}

	virtual CubeMT* copy() const {
		return new CubeMT(WHITE, _cRow, _cCol, _cFace);
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
			CubeMT* localCube = this->copy();
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

protected:
	int _cRow;
	int _cCol;
	int _cFace;

	std::vector<std::vector<std::vector<Color>>> _matrix;
	std::vector<std::vector<std::vector<Color>>> _initMatrix;
	std::vector<Rotation> _rotations;

	virtual void rotateFace(Faces face, bool clockwise) { }
	virtual void applyRotationInternal(Rotation r) { }

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

	std::string rotationsToString() {
		std::string retVal = "";
		for (Rotation r : _rotations) {
			retVal.append(rotationToString(r) + " ");
		}
		return retVal;
	}

	std::string rotationToString(Rotation r) {
		switch (r) {
		case U:  return "U";
		case D:  return "D";
		case R:  return "R";
		case L:  return "L";
		case F:  return "F";
		case B:  return "B";
		case UI: return "UI";
		case DI: return "DI";
		case RI: return "RI";
		case LI: return "LI";
		case FI: return "FI";
		case BI: return "BI";
		default: return "X";
		}
	}

	std::string colorToString(Color color, bool shortPrint = false) {
		if (shortPrint) {
			switch (color) {
			case RED:    return "R";
			case BLUE:   return "B";
			case ORANGE: return "O";
			case GREEN:  return "G";
			case WHITE:  return "W";
			case YELLOW: return "Y";
			default:     return "U";
			}
		}
		else {
			switch (color) {
			case RED:    return "RED";
			case BLUE:   return "BLUE";
			case ORANGE: return "ORANGE";
			case GREEN:  return "GREEN";
			case WHITE:  return "WHITE";
			case YELLOW: return "YELLOW";
			default:     return "UNKNOWN";
			}
		}
	}

	std::string faceToString(Faces face, bool shortPrint = false) {
		if (shortPrint) {
			switch (face) {
			case FRONT:  return "F";
			case RIGHT:  return "R";
			case BACK:   return "B";
			case LEFT:   return "L";
			case TOP:    return "T";
			case BOTTOM: return "B";
			default:     return "U";
			}
		}
		else {
			switch (face) {
			case FRONT:  return "FRONT";
			case RIGHT:  return "RIGHT";
			case BACK:   return "BACK";
			case LEFT:   return "LEFT";
			case TOP:    return "TOP";
			case BOTTOM: return "BOTTOM";
			default:     return "UNKNOWN";
			}
		}
	}
};

class Cube222MT : public CubeMT {
public:
	Cube222MT(Color initialColor = Color::WHITE, int cRow = 2, int cCol = 2, int cFace = 6) :
		CubeMT(initialColor, cRow, cCol, cFace) {
	}

	CubeMT* copy() const override {
		Cube222MT* newCube = new Cube222MT(*this);
		newCube->_matrix = this->_matrix;
		return newCube;
	}

protected:
	void rotateFace(Faces face, bool clockwise) override {
		if (clockwise) {
			Color temp = _matrix[face][0][0];
			_matrix[face][0][0] = _matrix[face][1][0];
			_matrix[face][1][0] = _matrix[face][1][1];
			_matrix[face][1][1] = _matrix[face][0][1];
			_matrix[face][0][1] = temp;
		}
		else {
			Color temp = _matrix[face][0][0];
			_matrix[face][0][0] = _matrix[face][0][1];
			_matrix[face][0][1] = _matrix[face][1][1];
			_matrix[face][1][1] = _matrix[face][1][0];
			_matrix[face][1][0] = temp;
		}
	}

	void applyRotationInternal(Rotation r) override {
		std::vector<Color> tempRow;
		std::vector<Color> tempColumn(_cCol);
		std::vector<Color> tempTop(_cCol);

		if (r == U || r == UI) {
			rotateFace(TOP, r == U);
			tempRow = _matrix[FRONT][0];
			if (r == U) {
				_matrix[FRONT][0] = _matrix[RIGHT][0];
				_matrix[RIGHT][0] = _matrix[BACK][0];
				_matrix[BACK][0] = _matrix[LEFT][0];
				_matrix[LEFT][0] = tempRow;
			}
			else {
				_matrix[FRONT][0] = _matrix[LEFT][0];
				_matrix[LEFT][0] = _matrix[BACK][0];
				_matrix[BACK][0] = _matrix[RIGHT][0];
				_matrix[RIGHT][0] = tempRow;
			}
		}
		else if (r == D || r == DI) {
			rotateFace(BOTTOM, r == D);
			tempRow = _matrix[FRONT][1];
			if (r == D) {
				_matrix[FRONT][1] = _matrix[LEFT][1];
				_matrix[LEFT][1] = _matrix[BACK][1];
				_matrix[BACK][1] = _matrix[RIGHT][1];
				_matrix[RIGHT][1] = tempRow;
			}
			else {
				_matrix[FRONT][1] = _matrix[RIGHT][1];
				_matrix[RIGHT][1] = _matrix[BACK][1];
				_matrix[BACK][1] = _matrix[LEFT][1];
				_matrix[LEFT][1] = tempRow;
			}
		}
		else if (r == L || r == LI) {
			rotateFace(LEFT, r == L);
			for (int i = 0; i < _cCol; i++) {
				tempColumn[i] = _matrix[TOP][i][0];
			}

			if (r == L) {
				for (int i = 0; i < _cCol; i++) {
					_matrix[TOP][i][0] = _matrix[BACK][1 - i][1];
					_matrix[BACK][1 - i][1] = _matrix[BOTTOM][i][0];
					_matrix[BOTTOM][i][0] = _matrix[FRONT][i][0];
					_matrix[FRONT][i][0] = tempColumn[i];
				}
			}
			else {
				for (int i = 0; i < _cCol; i++) {
					_matrix[TOP][i][0] = _matrix[FRONT][i][0];
					_matrix[FRONT][i][0] = _matrix[BOTTOM][i][0];
					_matrix[BOTTOM][i][0] = _matrix[BACK][1 - i][1];
					_matrix[BACK][1 - i][1] = tempColumn[i];
				}
			}
		}
		else if (r == R || r == RI) {
			rotateFace(RIGHT, r == R);

			for (int i = 0; i < _cCol; i++) {
				tempColumn[i] = _matrix[TOP][i][1];
			}

			if (r == R) {
				for (int i = 0; i < _cCol; i++) {
					_matrix[TOP][i][1] = _matrix[FRONT][i][1];
					_matrix[FRONT][i][1] = _matrix[BOTTOM][i][1];
					_matrix[BOTTOM][i][1] = _matrix[BACK][1 - i][0];
					_matrix[BACK][1 - i][0] = tempColumn[i];
				}
			}
			else {
				for (int i = 0; i < _cCol; i++) {
					_matrix[TOP][i][1] = _matrix[BACK][1 - i][0];
					_matrix[BACK][1 - i][0] = _matrix[BOTTOM][i][1];
					_matrix[BOTTOM][i][1] = _matrix[FRONT][i][1];
					_matrix[FRONT][i][1] = tempColumn[i];
				}
			}
		}
		else if (r == F || r == FI) {
			rotateFace(FRONT, r == F);

			for (int i = 0; i < _cCol; ++i) {
				tempTop[i] = _matrix[TOP][_cRow - 1][i];
			}

			if (r == F) {
				for (int i = 0; i < _cCol; ++i) {
					_matrix[TOP][_cRow - 1][i] = _matrix[LEFT][_cCol - 1 - i][_cRow - 1];
					_matrix[LEFT][_cCol - 1 - i][_cRow - 1] = _matrix[BOTTOM][0][i];
					_matrix[BOTTOM][0][i] = _matrix[RIGHT][i][0];
					_matrix[RIGHT][i][0] = tempTop[_cCol - 1 - i];
				}
			}
			else {
				for (int i = 0; i < _cCol; ++i) {
					_matrix[TOP][_cRow - 1][i] = _matrix[RIGHT][i][0];
					_matrix[RIGHT][i][0] = _matrix[BOTTOM][0][_cCol - 1 - i];
					_matrix[BOTTOM][0][_cCol - 1 - i] = _matrix[LEFT][_cCol - 1 - i][_cRow - 1];
					_matrix[LEFT][_cCol - 1 - i][_cRow - 1] = tempTop[i];
				}
			}
		}
		else if (r == B || r == BI) {
			rotateFace(BACK, r == B);

			for (int i = 0; i < _cCol; ++i) {
				tempTop[i] = _matrix[TOP][0][i];
			}

			if (r == B) {
				for (int i = 0; i < _cCol; ++i) {
					_matrix[TOP][0][i] = _matrix[LEFT][_cCol - 1 - i][0];
					_matrix[LEFT][_cCol - 1 - i][0] = _matrix[BOTTOM][_cRow - 1][_cCol - 1 - i];
					_matrix[BOTTOM][_cRow - 1][_cCol - 1 - i] = _matrix[RIGHT][i][_cRow - 1];
					_matrix[RIGHT][i][_cRow - 1] = tempTop[_cCol - 1 - i];
				}
			}
			else {
				for (int i = 0; i < _cCol; ++i) {
					_matrix[TOP][0][i] = _matrix[RIGHT][i][_cRow - 1];
					_matrix[RIGHT][i][_cRow - 1] = _matrix[BOTTOM][_cRow - 1][_cCol - 1 - i];
					_matrix[BOTTOM][_cRow - 1][_cCol - 1 - i] = _matrix[LEFT][_cCol - 1 - i][0];
					_matrix[LEFT][_cCol - 1 - i][0] = tempTop[i];
				}
			}
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

	std::cout << "2x2x2 Cube (Multi-Threaded Solver):" << std::endl;
	cube.printCube();

	auto startTime = std::chrono::steady_clock::now();
	cube.dfsParallel(1, startTime);

	cube.printCube();

	return 0;
}
