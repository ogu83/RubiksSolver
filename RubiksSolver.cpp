#include "RubiksSolver.h"

using namespace std;

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED }; 
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE }; 
enum Rotation { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI, ROTATION_NONE };

std::map<char, Color> charToColor = {
	{'R', RED}, {'B', BLUE}, {'O', ORANGE}, {'G', GREEN}, {'W', WHITE}, {'Y', YELLOW}
};

std::map<std::string, Faces> tagToFace = {
	{"-ft", TOP}, {"-ff", FRONT}, {"-fr", RIGHT}, {"-fb", BOTTOM}, {"-fbk", BACK}, {"-fl", LEFT}
};

// Inverse rotation lookup table for move pruning
const Rotation inverseRotation[] = { UI, DI, RI, LI, FI, BI, U, D, R, L, F, B, ROTATION_NONE };

// Face group lookup - moves on the same face or opposite faces can be optimized
// 0 = U/D axis, 1 = R/L axis, 2 = F/B axis
const int rotationAxis[] = { 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, -1 };

// Base face for each rotation (without considering inverse)
const int rotationBaseFace[] = { 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, -1 };

class Cube {
public:
	Cube(Color initialColor, int cRow, int cCol, int cFace)
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
		else {
			std::cerr << "Index out of bounds error." << std::endl;
		}
	}

	Color getColor(Faces face, int row, int col) const {
		if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
			return _matrix[face][row][col];
		}
		else {
			std::cerr << "Index out of bounds error." << std::endl;
			return Color::WHITE;
		}
	}

	virtual void applyRotation(Rotation r) {
		_rotations.push_back(r);
	}

	virtual void undoRotation(Rotation r) {
		applyRotationInternal(inverseRotation[r]);
		if (!_rotations.empty()) {
			_rotations.pop_back();
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
		for (size_t f = 0; f < _cFace/2; ++f) {
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

	int heuristic() const {
		int misplaced = 0;
		for (size_t f = 0; f < _cFace; ++f) {
			const auto& face = _matrix[f];
			const Color referenceColor = face[0][0];
			for (size_t i = 0; i < _cCol; ++i) {
				for (size_t j = 0; j < _cRow; ++j) {
					if (face[i][j] != referenceColor) {
						misplaced++;
					}
				}
			}
		}
		return misplaced / 8;
	}

	std::string getStateHash() const {
		std::string hash;
		hash.reserve(_cFace * _cRow * _cCol);
		for (int f = 0; f < _cFace; f++) {
			for (int i = 0; i < _cRow; i++) {
				for (int j = 0; j < _cCol; j++) {
					hash += static_cast<char>('0' + _matrix[f][i][j]);
				}
			}
		}
		return hash;
	}

	static bool isRedundantMove(Rotation lastMove, Rotation currentMove) {
		if (lastMove == ROTATION_NONE) return false;
		if (inverseRotation[lastMove] == currentMove) return true;
		if (lastMove == currentMove) return true;
		if (rotationAxis[lastMove] == rotationAxis[currentMove]) {
			int lastBase = rotationBaseFace[lastMove];
			int currentBase = rotationBaseFace[currentMove];
			if (lastBase > currentBase) return true;
		}
		return false;
	}

	void applySolution(const std::vector<Rotation>& solution) {
		for (Rotation move : solution) {
			applyRotation(move);
		}
	}

	virtual Cube* copy() const {
		return new Cube(WHITE, _cRow, _cCol, _cFace);
	}

	virtual void idaStar(const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		if (isSolved()) {
			std::cout << "Already solved!\n";
			return;
		}

		_solutionFound = false;
		_solution.clear();
		_nodesExplored = 0;

		int depthLimit = heuristic();
		if (depthLimit == 0) depthLimit = 1;

		while (!_solutionFound && depthLimit <= 20) {
			std::cout << "Searching depth " << depthLimit << "...\n";
			_visitedStates.clear();
			
			std::vector<Rotation> path;
			idaStarRecursive(0, depthLimit, ROTATION_NONE, path, begin_time);
			
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
				applySolution(_solution);
				return;
			}
			depthLimit++;
		}
		std::cout << "No solution found within depth limit.\n";
	}

	virtual void dfs(int depth = 1, const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		idaStar(begin_time);
	}

	virtual void dfsLegacy(int depth = 1, const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		if (isSolved()) {
			return;
		}

		static const std::vector<Rotation> allRotations = { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };
		std::vector<Rotation> currentPath;
		std::vector<std::vector<Rotation>> potentialSolutions;

		generateCombinations(allRotations, depth, currentPath, potentialSolutions);
		std::cout << potentialSolutions.size() << " combinations testing.\n";

		auto endTime = std::chrono::steady_clock::now();
		std::chrono::duration<double> timeTaken = endTime - begin_time;

		for (const auto& solution : potentialSolutions) {
			applySolution(solution);
			if (isSolved()) {
				std::cout << "Solved in " << timeTaken.count() << " seconds.\n";
				std::cout << "Solution: ";
				for (Rotation move : solution) {
					std::cout << rotationToString(move) << " ";
				}
				std::cout << "\n";
				return;
			}
			reset();
		}

		std::cout << timeTaken.count() << " seconds elapsed.\nIncreasing depth to " << depth + 1 << ". Continue search...\n";
		dfsLegacy(depth + 1, begin_time);
	}

protected:
	int _cRow;
	int _cCol;
	int _cFace;

	std::vector<std::vector<std::vector<Color>>> _matrix;
	std::vector<std::vector<std::vector<Color>>> _initMatrix;
	std::vector<Rotation> _rotations;

	bool _solutionFound = false;
	std::vector<Rotation> _solution;
	std::unordered_set<std::string> _visitedStates;
	size_t _nodesExplored = 0;

	virtual void applyRotationInternal(Rotation r) { }
	virtual void rotateFace(Faces face, bool clockwise) { }

	bool idaStarRecursive(int currentDepth, int depthLimit, Rotation lastMove, 
		std::vector<Rotation>& path,
		const std::chrono::time_point<std::chrono::steady_clock>& begin_time) {
		_nodesExplored++;
		
		if (isSolved()) {
			_solutionFound = true;
			_solution = path;
			return true;
		}

		int h = heuristic();
		if (currentDepth + h > depthLimit) {
			return false;
		}

		std::string stateHash = getStateHash();
		if (_visitedStates.find(stateHash) != _visitedStates.end()) {
			return false;
		}
		_visitedStates.insert(stateHash);

		static const std::vector<Rotation> allRotations = { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };
		
		for (Rotation r : allRotations) {
			if (isRedundantMove(lastMove, r)) {
				continue;
			}

			applyRotation(r);
			path.push_back(r);

			if (idaStarRecursive(currentDepth + 1, depthLimit, r, path, begin_time)) {
				return true;
			}

			undoRotation(r);
			path.pop_back();
		}

		return false;
	}

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

class Cube222 : public Cube {
public:
	Cube222(Color initialColor = Color::WHITE, int cRow = 2, int cCol = 2, int cFace = 6) :
		Cube(initialColor, cRow, cCol, cFace) {
	}

	Cube* copy() const override {
		Cube222* newCube = new Cube222(*this);
		newCube->_matrix = this->_matrix;
		return newCube;
	}

	void applyRotation(Rotation r) override {
		applyRotationInternal(r);
		Cube::applyRotation(r);
	}

	void undoRotation(Rotation r) override {
		applyRotationInternal(inverseRotation[r]);
		if (!_rotations.empty()) {
			_rotations.pop_back();
		}
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
	Cube222 cube;

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

	std::cout << "2x2x2 Cube:" << std::endl;
	cube.printCube();

	cube.dfs();

	cube.printCube();

	return 0;
}