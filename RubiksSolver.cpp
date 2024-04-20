#include "RubiksSolver.h"

using namespace std;

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE };
enum Rotation { U, D, R, L, UI, DI, RI, LI };

std::map<char, Color> charToColor = {
	{'R', RED}, {'B', BLUE}, {'O', ORANGE}, {'G', GREEN}, {'W', WHITE}, {'Y', YELLOW}
};

std::map<std::string, Faces> tagToFace = {
	{"-ft", TOP}, {"-ff", FRONT}, {"-fr", RIGHT}, {"-fb", BOTTOM}, {"-fbk", BACK}, {"-fl", LEFT}
};

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
		_rotations.clear();
		setColor(FRONT, BLUE);
		setColor(RIGHT, RED);
		setColor(TOP, YELLOW);
		setColor(BOTTOM, WHITE);
		setColor(BACK, GREEN);
		setColor(LEFT, ORANGE);
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
	/// Check if tich cube is solved or not
	/// </summary>
	/// <returns>Solved or Not</returns>
	inline bool isSolved() const {
		for (const auto& face : _matrix) {
			const Color firstColor = face[0][0];
			for (const auto& row : face) {
				if (std::any_of(row.begin(), row.end(), [firstColor](Color color) { return color != firstColor; })) {
					return false;
				}
			}
		}
		return true;
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
		return new Cube(WHITE, _cRow, _cCol, _cFace); // Return a new Cube allocated with new
	}

	/// <summary>
	/// Depth First Search For Solve The Cube
	/// </summary>
	/// <param name="depth">Depth</param>
	/// <param name="begin_time">Start Time</param>
	virtual void dfs(int depth = 1, const std::chrono::time_point<std::chrono::steady_clock>& begin_time = std::chrono::steady_clock::now()) {
		if (isSolved()) {
			return;
		}

		static const std::vector<Rotation> allRotations = { U, D, R, L, UI, DI, RI, LI };
		std::vector<Rotation> currentPath;
		std::vector<std::vector<Rotation>> potentialSolutions;

		// Generate all combinations of moves up to the given depth
		generateCombinations(allRotations, depth, currentPath, potentialSolutions);

		//auto startTime = std::chrono::steady_clock::now();
		auto endTime = std::chrono::steady_clock::now();
		std::chrono::duration<double> timeTaken = endTime - begin_time;

		for (const auto& solution : potentialSolutions) {
			std::unique_ptr<Cube> testCube(copy());  // Use smart pointers to manage memory
			testCube->applySolution(solution);
			
			//std::cout << "Testing: ";
			//for (Rotation move : solution) {
			//	std::cout << rotationToString(move) << " ";
			//}
			//std::cout << "\n";

			//testCube->printCube(true);

			if (testCube->isSolved()) {
				std::cout << "Solved in " << timeTaken.count() << " seconds\n";
				std::cout << "Solution: ";
				for (Rotation move : solution) {
					std::cout << rotationToString(move) << " ";
				}
				std::cout << "\n";
				return;
			}
		}

		std::cout << timeTaken.count() << " time spend. Increasing depth to " << depth + 1 << ". Continue search...\n";
		dfs(depth + 1, begin_time);
	}

protected:

	int _cRow;
	int _cCol;
	int _cFace;

	std::vector<std::vector<std::vector<Color>>> _matrix;
	std::vector<Rotation> _rotations;

	/// <summary>
	/// Rotate One face of the Cube
	/// </summary>
	/// <param name="face">Face</param>
	/// <param name="clockwise">ClockWise or Counter Clock Wise</param>
	virtual void rotateFace(Faces face, bool clockwise) { };

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
		case U:		return "U";
		case D:		return "D";
		case R:		return "R";
		case L:		return "L";
		case UI:	return "UI";
		case DI:	return "DI";
		case RI:	return "RI";
		case LI:	return "LI";
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

	//Cube copy() const override {
	//	Cube222 newCube(*this);  // Create a copy of this cube
	//	newCube._matrix = _matrix;  // Copy the matrix explicitly if needed
	//	return newCube;
	//}

	Cube* copy() const override {
		Cube222* newCube = new Cube222(*this);  // Dynamically allocate a new Cube222
		newCube->_matrix = this->_matrix;       // Explicitly copy the matrix
		return newCube;                         // Return as a pointer to Cube
	}

	/// <summary>
	/// Make a rotation
	/// </summary>
	/// <param name="r">Rotation</param>
	void applyRotation(Rotation r) override {
		// Allocate temporary storage for one row or one column since cube is 2x2
		std::array<Color, 2> tempStorage;

		if (r == U || r == UI) {
			rotateFace(TOP, r == U);
			if (r == U) {  // Clockwise
				std::swap_ranges(_matrix[FRONT][0].begin(), _matrix[FRONT][0].end(), _matrix[RIGHT][0].begin());
				std::swap_ranges(_matrix[RIGHT][0].begin(), _matrix[RIGHT][0].end(), _matrix[BACK][0].begin());
				std::swap_ranges(_matrix[BACK][0].begin(), _matrix[BACK][0].end(), _matrix[LEFT][0].begin());
				std::swap_ranges(_matrix[LEFT][0].begin(), _matrix[LEFT][0].end(), _matrix[FRONT][0].begin());
			}
			else {  // Counter-clockwise (UI)
				std::swap_ranges(_matrix[FRONT][0].begin(), _matrix[FRONT][0].end(), _matrix[LEFT][0].begin());
				std::swap_ranges(_matrix[LEFT][0].begin(), _matrix[LEFT][0].end(), _matrix[BACK][0].begin());
				std::swap_ranges(_matrix[BACK][0].begin(), _matrix[BACK][0].end(), _matrix[RIGHT][0].begin());
				std::swap_ranges(_matrix[RIGHT][0].begin(), _matrix[RIGHT][0].end(), _matrix[FRONT][0].begin());
			}
		}
		else if (r == D || r == DI) {
			rotateFace(BOTTOM, r == D);
			if (r == D) {  // Clockwise
				std::swap_ranges(_matrix[FRONT][1].begin(), _matrix[FRONT][1].end(), _matrix[RIGHT][1].begin());
				std::swap_ranges(_matrix[RIGHT][1].begin(), _matrix[RIGHT][1].end(), _matrix[BACK][1].begin());
				std::swap_ranges(_matrix[BACK][1].begin(), _matrix[BACK][1].end(), _matrix[LEFT][1].begin());
				std::swap_ranges(_matrix[LEFT][1].begin(), _matrix[LEFT][1].end(), _matrix[FRONT][1].begin());
			}
			else {  // Counter-clockwise (DI)
				std::swap_ranges(_matrix[FRONT][1].begin(), _matrix[FRONT][1].end(), _matrix[LEFT][1].begin());
				std::swap_ranges(_matrix[LEFT][1].begin(), _matrix[LEFT][1].end(), _matrix[BACK][1].begin());
				std::swap_ranges(_matrix[BACK][1].begin(), _matrix[BACK][1].end(), _matrix[RIGHT][1].begin());
				std::swap_ranges(_matrix[RIGHT][1].begin(), _matrix[RIGHT][1].end(), _matrix[FRONT][1].begin());
			}
		}
		else if (r == L || r == LI) {
			rotateFace(LEFT, r == L);
			for (int i = 0; i < _cCol; i++) {
				tempStorage[i] = _matrix[TOP][i][0];  // Store the left column of the top face
			}

			if (r == L) { // Clockwise
				for (int i = 0; i < _cCol; i++) {
					std::swap(_matrix[TOP][i][0], _matrix[BACK][1 - i][1]);
					std::swap(_matrix[BACK][1 - i][1], _matrix[BOTTOM][i][0]);
					std::swap(_matrix[BOTTOM][i][0], _matrix[FRONT][i][0]);
					_matrix[FRONT][i][0] = tempStorage[i];
				}
			}
			else { // Counter-clockwise (LI)
				for (int i = 0; i < _cCol; i++) {
					std::swap(_matrix[TOP][i][0], _matrix[FRONT][i][0]);
					std::swap(_matrix[FRONT][i][0], _matrix[BOTTOM][i][0]);
					std::swap(_matrix[BOTTOM][i][0], _matrix[BACK][1 - i][1]);
					_matrix[BACK][1 - i][1] = tempStorage[i];
				}
			}
		}
		else if (r == R || r == RI) {
			rotateFace(RIGHT, r == R);
			for (int i = 0; i < _cCol; i++) {
				tempStorage[i] = _matrix[TOP][i][1];  // Store the right column of the top face
			}

			if (r == R) { // Clockwise
				for (int i = 0; i < _cCol; i++) {
					std::swap(_matrix[TOP][i][1], _matrix[FRONT][i][1]);
					std::swap(_matrix[FRONT][i][1], _matrix[BOTTOM][i][1]);
					std::swap(_matrix[BOTTOM][i][1], _matrix[BACK][1 - i][0]);
					_matrix[BACK][1 - i][0] = tempStorage[i];
				}
			}
			else { // Counter-clockwise (RI)
				for (int i = 0; i < _cCol; i++) {
					std::swap(_matrix[TOP][i][1], _matrix[BACK][1 - i][0]);
					std::swap(_matrix[BACK][1 - i][0], _matrix[BOTTOM][i][1]);
					std::swap(_matrix[BOTTOM][i][1], _matrix[FRONT][i][1]);
					_matrix[FRONT][i][1] = tempStorage[i];
				}
			}
		}

		Cube::applyRotation(r);  // Logging rotation
	}


protected:
	/// <summary>
	/// Rotate One face of the Cube
	/// </summary>
	/// <param name="face">Face</param>
	/// <param name="clockwise">ClockWise or Counter Clock Wise</param>
	void rotateFace(Faces face, bool clockwise) override {
		if (clockwise) {
			// Rotate face 90 degrees clockwise
			Color temp = _matrix[face][0][0];
			_matrix[face][0][0] = _matrix[face][1][0];
			_matrix[face][1][0] = _matrix[face][1][1];
			_matrix[face][1][1] = _matrix[face][0][1];
			_matrix[face][0][1] = temp;
		}
		else {
			// Rotate face 90 degrees counterclockwise
			Color temp = _matrix[face][0][0];
			_matrix[face][0][0] = _matrix[face][0][1];
			_matrix[face][0][1] = _matrix[face][1][1];
			_matrix[face][1][1] = _matrix[face][1][0];
			_matrix[face][1][0] = temp;
		}

		Cube::rotateFace(face, clockwise);
	}
};

int main(int argc, char* argv[]) {
	Cube222 cube;
	//cube.applyRotation(R);

	for (int i = 1; i < argc; i += 2) {
		if (i + 1 < argc) {
			std::string tag = argv[i];
			std::string values = argv[i + 1];
			std::vector<Color> colors;

			// Convert string of colors to vector of Color enums
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

	std::cout << "2x2x2 Cube:" << std::endl;
	cube.printCube(true);
	cube.dfs();
	return 0;
};