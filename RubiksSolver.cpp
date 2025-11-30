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
	/// <summary>
	/// Constructor of The Cube
	/// </summary>
	/// <param name="initialColor">Initial Color</param>
	/// <param name="cRow">Row Count For Each Face</param>
	/// <param name="cCol">Col Count For Each Face</param>
	/// <param name="cFace">Face Count</param>
	Cube(Color initialColor, int cRow, int cCol, int cFace)
		: _cRow(cRow), _cCol(cCol), _cFace(cFace),
		_matrix(cFace, std::vector<std::vector<Color>>(cCol, std::vector<Color>(cRow, initialColor))) {

		setColorsToInitState();
	}

	/// <summary>
	/// Goto init state
	/// </summary>
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

	/// <summary>
	/// Function to set the colors of the face
	/// </summary>
	/// <param name="face">Face</param>
	/// <param name="colors">Colors</param>
	void setColor(Faces face, const std::vector<Color>& colors) {
		for (int i = 0; i < _cRow; ++i) {
			for (int j = 0; j < _cCol; ++j) {
				int idx = i * _cCol + j;  // Flatten the row/col to index
				if (idx < colors.size()) {
					_matrix[face][i][j] = colors[idx];
				}
			}
		}
	}

	/// <summary>
	/// Function to set the color of the face
	/// </summary>
	/// <param name="face">Face</param>
	/// <param name="color">Color</param>
	void setColor(Faces face, Color color) {
		for (int r = 0; r < _cRow; r++) {
			for (int c = 0; c < _cCol; c++) {
				setColor(face, r, c, color);
			}
		}
	}

	/// <summary>
	/// Function to set the color of a specific cell
	/// </summary>
	/// <param name="face">Face</param>
	/// <param name="row">Row</param>
	/// <param name="col">Column</param>
	/// <param name="color">Color</param>
	void setColor(Faces face, int row, int col, Color color) {
		if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
			_matrix[face][row][col] = color;
		}
		else {
			std::cerr << "Index out of bounds error." << std::endl;
		}
	}

	/// <summary>
	/// Function to get the color of a specific cell
	/// </summary>
	/// <param name="face">Face</param>
	/// <param name="row">Row</param>
	/// <param name="col">Column</param>
	/// <param name="color">Color</param>
	/// <returns>Color</returns>
	Color getColor(Faces face, int row, int col) const {
		if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
			return _matrix[face][row][col];
		}
		else {
			std::cerr << "Index out of bounds error." << std::endl;
			return Color::WHITE;  // Default return
		}
	}

	/// <summary>
	/// Make a rotation
	/// </summary>
	/// <param name="r">Rotation</param>
	virtual void applyRotation(Rotation r) {
		_rotations.push_back(r);
	}

	/// <summary>
	/// Undo a rotation (apply the inverse)
	/// </summary>
	/// <param name="r">Rotation to undo</param>
	virtual void undoRotation(Rotation r) {
		applyRotationInternal(inverseRotation[r]);
		if (!_rotations.empty()) {
			_rotations.pop_back();
		}
	}

	/// <summary>
	/// Utility to print the cube's configuration
	/// </summary>
	/// <param name="shortPrint"></param>
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

	/// <summary>
	/// Check if this cube is solved or not
	/// </summary>
	/// <returns>Solved or Not</returns>
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

	/// <summary>
	/// Heuristic function for IDA* - counts misplaced stickers
	/// Returns an admissible heuristic (never overestimates)
	/// </summary>
	/// <returns>Estimated minimum moves to solve</returns>
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
		// Each move affects at most 8 stickers (4 on the face + 4 on adjacent faces for 2x2)
		// Using /8 makes this admissible but can be tuned
		return misplaced / 8;
	}

	/// <summary>
	/// Generate a unique hash for the current cube state
	/// Used for memoization to avoid revisiting states
	/// </summary>
	/// <returns>String hash of the cube state</returns>
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

	/// <summary>
	/// Check if a move is redundant given the last move
	/// Prunes moves that cancel out or can be combined
	/// </summary>
	/// <param name="lastMove">The previous move</param>
	/// <param name="currentMove">The move to check</param>
	/// <returns>True if the move is redundant</returns>
	static bool isRedundantMove(Rotation lastMove, Rotation currentMove) {
		if (lastMove == ROTATION_NONE) return false;
		
		// Don't allow inverse immediately after a move (they cancel out)
		// e.g., U followed by UI is wasteful
		if (inverseRotation[lastMove] == currentMove) return true;
		
		// Don't allow same move twice in a row (should use double move notation)
		// e.g., U U should be U2 in optimal solving
		if (lastMove == currentMove) return true;
		
		// For moves on the same axis, enforce an ordering to avoid duplicates
		// e.g., U D is same as D U, so only allow one ordering
		if (rotationAxis[lastMove] == rotationAxis[currentMove]) {
			// Allow the move only if it's "larger" to enforce consistent ordering
			int lastBase = rotationBaseFace[lastMove];
			int currentBase = rotationBaseFace[currentMove];
			if (lastBase > currentBase) return true;
		}
		
		return false;
	}

	/// <summary>
	/// Apply A solution to this cube
	/// </summary>
	/// <param name="solution">A solution array from rotations enum elements</param>
	void applySolution(const std::vector<Rotation>& solution) {
		for (Rotation move : solution) {
			applyRotation(move);
		}
	}

	/// <summary>
	/// Clone the cube
	/// </summary>
	/// <returns>The Cube</returns>
	virtual Cube* copy() const {
		return new Cube(WHITE, _cRow, _cCol, _cFace);
	}

	/// <summary>
	/// IDA* (Iterative Deepening A*) search - much faster than brute force DFS
	/// Uses heuristic to prune branches that can't lead to a solution
	/// </summary>
	/// <param name="begin_time">Start time for timing</param>
	virtual void idaStar(const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		if (isSolved()) {
			std::cout << "Already solved!\n";
			return;
		}

		_solutionFound = false;
		_solution.clear();
		_nodesExplored = 0;

		int depthLimit = heuristic();  // Start with heuristic estimate
		if (depthLimit == 0) depthLimit = 1;

		while (!_solutionFound && depthLimit <= 20) {  // Max depth limit for safety
			std::cout << "Searching depth " << depthLimit << "...\n";
			_visitedStates.clear();  // Clear visited states for each depth iteration
			
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
				
				// Apply the solution
				applySolution(_solution);
				return;
			}
			
			depthLimit++;
		}

		std::cout << "No solution found within depth limit.\n";
	}

	/// <summary>
	/// Original DFS method (kept for backwards compatibility)
	/// Now calls the optimized IDA* instead
	/// </summary>
	virtual void dfs(int depth = 1, const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		// Use the new optimized IDA* search
		idaStar(begin_time);
	}

	/// <summary>
	/// Legacy brute-force DFS (kept for comparison/testing)
	/// </summary>
	virtual void dfsLegacy(int depth = 1, const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		if (isSolved()) {
			return;
		}

		static const std::vector<Rotation> allRotations = { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };
		std::vector<Rotation> currentPath;
		std::vector<std::vector<Rotation>> potentialSolutions;

		// Generate all combinations of moves up to the given depth
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

	// IDA* search state
	bool _solutionFound = false;
	std::vector<Rotation> _solution;
	std::unordered_set<std::string> _visitedStates;
	size_t _nodesExplored = 0;

	/// <summary>
	/// Internal rotation application (without tracking)
	/// </summary>
	virtual void applyRotationInternal(Rotation r) { }

	/// <summary>
	/// Rotate One face of the Cube
	/// </summary>
	/// <param name="face">Face</param>
	/// <param name="clockwise">ClockWise or Counter Clock Wise</param>
	virtual void rotateFace(Faces face, bool clockwise) { }; 

	/// <summary>
	/// Recursive IDA* search
	/// </summary>
	bool idaStarRecursive(int currentDepth, int depthLimit, Rotation lastMove, 
		std::vector<Rotation>& path,
		const std::chrono::time_point<std::chrono::steady_clock>& begin_time) {
		_nodesExplored++;
		
		// Check if solved
		if (isSolved()) {
			_solutionFound = true;
			_solution = path;
			return true;
		}

		// Pruning: if current depth + heuristic > limit, don't explore further
		int h = heuristic();
		if (currentDepth + h > depthLimit) {
			return false;
		}

		// State hashing to avoid revisiting
		std::string stateHash = getStateHash();
		if (_visitedStates.find(stateHash) != _visitedStates.end()) {
			return false;  // Already visited this state
		}
		_visitedStates.insert(stateHash);

		// Try all rotations with pruning
		static const std::vector<Rotation> allRotations = { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };
		
		for (Rotation r : allRotations) {
			// Prune redundant moves
			if (isRedundantMove(lastMove, r)) {
				continue;
			}

			// Apply move
			applyRotation(r);
			path.push_back(r);

			// Recurse
			if (idaStarRecursive(currentDepth + 1, depthLimit, r, path, begin_time)) {
				return true;
			}

			// Undo move (backtrack)
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

	/// <summary>
	/// Convert Rotations Log to string
	/// </summary>
	/// <returns>Rotation String</returns>
	std::string rotationsToString() {
		std::string retVal = "";
		for (Rotation r : _rotations) {
			retVal.append(rotationToString(r) + " ");
		}
		return retVal;
	}

	/// <summary>
	/// Convert Rotation enum to string
	/// </summary>
	/// <param name="r">Rotation</param>
	/// <returns>String On the Rotation in the Enum</returns>
	std::string rotationToString(Rotation r) {
		switch (r)
		{
		case U: 	return "U";
		case D: 	return "D";
		case R: 	return "R";
		case L: 	return "L";
		case F: 	return "F";
		case B: 	return "B";
		case UI: 	return "UI";
		case DI: 	return "DI";
		case RI: 	return "RI";
		case LI: 	return "LI";
		case FI: 	return "FI";
		case BI: 	return "BI";
		default:	return "X";
		}
	}

	/// <summary>
	/// Convert Color enum to string
	/// </summary>
	/// <param name="color">Color</param>
	/// <param name="shortPrint">Short Print: For Small Console Output</param>
	/// <returns>String Of the Color Enum</returns>
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

	/// <summary>
	/// Convert Faces enum to string
	/// </summary>
	/// <param name="face">Face</param>
	/// <param name="shortPrint">Short Print: For Small Console Output</param>
	/// <returns>String Of the Faces Enum</returns>
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

	/// <summary>
	/// Constructor of The Cube 2x2x2
	/// </summary>
	/// <param name="initialColor">Initial Color</param>
	/// <param name="cRow">Row Count For Each Face</param>
	/// <param name="cCol">Col Count For Each Face</param>
	/// <param name="cFace">Face Count</param>
	Cube222(Color initialColor = Color::WHITE, int cRow = 2, int cCol = 2, int cFace = 6) :
		Cube(initialColor, cRow, cCol, cFace) {
	}

	Cube* copy() const override {
		Cube222* newCube = new Cube222(*this);
		newCube->_matrix = this->_matrix;
		return newCube;
	}

	/// <summary>
	/// Make a rotation
	/// </summary>
	/// <param name="r">Rotation</param>
	void applyRotation(Rotation r) override {
		applyRotationInternal(r);
		Cube::applyRotation(r);
	}

	/// <summary>
	/// Undo a rotation
	/// </summary>
	void undoRotation(Rotation r) override {
		applyRotationInternal(inverseRotation[r]);
		if (!_rotations.empty()) {
			_rotations.pop_back();
		}
	}

protected:
	/// <summary>
	/// Internal rotation logic without tracking
	/// </summary>
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
	};