#include "RubiksSolver.h"

using namespace std;

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT };
enum Rotation { U, D, R, L, UI, DI, RI, LI };

class Cube {
public:
	Cube(Color initialColor, int cRow, int cCol, int cFace)
		: _cRow(cRow), _cCol(cCol), _cFace(cFace),
		_matrix(cFace, std::vector<std::vector<Color>>(cCol, std::vector<Color>(cRow, initialColor))) {

		setColorsToInitState();
	}

	//goto init state
	void setColorsToInitState() {
		_rotations.clear();
		setColor(FRONT, BLUE);
		setColor(RIGHT, RED);
		setColor(TOP, YELLOW);
		setColor(BOTTOM, WHITE);
		setColor(BACK, GREEN);
		setColor(LEFT, ORANGE);
	}

	//make a rotation
	virtual void applyRotation(Rotation r) {
		_rotations.push_back(r);
	}

	// Utility to print the cube's configuration
	void printCube(bool shortPrint = false) {
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

protected:
	int _cRow;
	int _cCol;
	int _cFace;
	std::vector<std::vector<std::vector<Color>>> _matrix;
	std::vector<Rotation> _rotations;

	virtual void rotateFace(Faces face, bool clockwise) { };

	// Function to set the color of the face
	void setColor(Faces face, Color color) {
		for (int r = 0; r < _cRow; r++) {
			for (int c = 0; c < _cCol; c++) {
				setColor(face, r, c, color);
			}
		}
	}

	// Function to set the color of a specific cell
	void setColor(Faces face, int row, int col, Color color) {
		if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
			_matrix[face][row][col] = color;
		}
		else {
			std::cerr << "Index out of bounds error." << std::endl;
		}
	}

	// Function to get the color of a specific cell
	Color getColor(Faces face, int row, int col) const {
		if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
			return _matrix[face][row][col];
		}
		else {
			std::cerr << "Index out of bounds error." << std::endl;
			return Color::WHITE;  // Default return
		}
	}

	// Convert Rotations Log to string
	std::string rotationsToString() {
		std::string retVal = "";
		for (Rotation r : _rotations) {
			retVal.append(rotationToString(r) + " ");
		}
		return retVal;
	}

	// Convert Rotation enum to string
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

	// Convert Color enum to string
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

	// Convert Faces enum to string
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

	void applyRotation(Rotation r) {
		Cube::applyRotation(r);
		std::vector<Color> tempRow;
		std::vector<Color> tempColumn(_cCol);
		if (r == U || r == UI) {
			// Rotate the top face
			rotateFace(TOP, r == U);
			// Cycle the top rows
			tempRow = _matrix[FRONT][0]; // Copy front top row
			if (r == U) { // Clockwise
				_matrix[FRONT][0] = _matrix[RIGHT][0];
				_matrix[RIGHT][0] = _matrix[BACK][0];
				_matrix[BACK][0] = _matrix[LEFT][0];
				_matrix[LEFT][0] = tempRow;
			}
			else { // Counter-clockwise (UI)
				_matrix[FRONT][0] = _matrix[LEFT][0];
				_matrix[LEFT][0] = _matrix[BACK][0];
				_matrix[BACK][0] = _matrix[RIGHT][0];
				_matrix[RIGHT][0] = tempRow;
			}
		}
		else if (r == D || r == DI) {
			// Rotate the bottom face
			rotateFace(BOTTOM, r == D);
			// Cycle te bottom rows
			tempRow = _matrix[FRONT][1]; // Copy front down row
			if (r == D) { // Clockwise
				_matrix[FRONT][1] = _matrix[RIGHT][1];
				_matrix[RIGHT][1] = _matrix[BACK][1];
				_matrix[BACK][1] = _matrix[LEFT][1];
				_matrix[LEFT][1] = tempRow;
			}
			else { // Counter-clockwise
				_matrix[FRONT][1] = _matrix[LEFT][1];
				_matrix[LEFT][1] = _matrix[BACK][1];
				_matrix[BACK][1] = _matrix[RIGHT][1];
				_matrix[RIGHT][1] = tempRow;
			}
		}
		else if (r == L || r == LI) {
			// Rotate the left face
			rotateFace(LEFT, r == L);
			// Cycling the columns for L or LI rotation
			for (int i = 0; i < _cCol; i++) {  // Since it's a 2x2 cube
				tempColumn[i] = _matrix[TOP][i][0];  // Store the left column of the top face
			}

			if (r == L) { // Clockwise
				for (int i = 0; i < _cCol; i++) {
					_matrix[TOP][i][0] = _matrix[BACK][1 - i][1];  // Back to top, flipped vertically
					_matrix[BACK][1 - i][1] = _matrix[BOTTOM][i][0];  // Bottom to back, flipped vertically
					_matrix[BOTTOM][i][0] = _matrix[FRONT][i][0];  // Front to bottom
					_matrix[FRONT][i][0] = tempColumn[i];  // Top to front
				}
			}
			else { // Counter-clockwise
				for (int i = 0; i < _cCol; i++) {
					_matrix[TOP][i][0] = _matrix[FRONT][i][0];  // Front to top
					_matrix[FRONT][i][0] = _matrix[BOTTOM][i][0];  // Bottom to front
					_matrix[BOTTOM][i][0] = _matrix[BACK][1 - i][1];  // Back to bottom, flipped vertically
					_matrix[BACK][1 - i][1] = tempColumn[i];  // Top to back, flipped vertically
				}
			}
		}
		else if (r == R || r == RI) {
			// Rotate the right face
			rotateFace(RIGHT, r == R);

			// Cycling the columns for R or RI rotation
			for (int i = 0; i < _cCol; i++) {
				tempColumn[i] = _matrix[TOP][i][1];  // Store the right column of the top face
			}

			if (r == R) { // Clockwise
				for (int i = 0; i < _cCol; i++) {
					_matrix[TOP][i][1] = _matrix[FRONT][i][1];  // Front to top
					_matrix[FRONT][i][1] = _matrix[BOTTOM][i][1];  // Bottom to front
					_matrix[BOTTOM][i][1] = _matrix[BACK][1 - i][0];  // Back (flipped vertically) to bottom
					_matrix[BACK][1 - i][0] = tempColumn[i];  // Top to back (flipped vertically)
				}
			}
			else { // Counter-clockwise (RI)
				for (int i = 0; i < _cCol; i++) {
					_matrix[TOP][i][1] = _matrix[BACK][1 - i][0];  // Back (flipped vertically) to top
					_matrix[BACK][1 - i][0] = _matrix[BOTTOM][i][1];  // Bottom to back (flipped vertically)
					_matrix[BOTTOM][i][1] = _matrix[FRONT][i][1];  // Front to bottom
					_matrix[FRONT][i][1] = tempColumn[i];  // Top to front
				}
			}
		}
	}

protected:
	void rotateFace(Faces face, bool clockwise) {
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
	}
};

int main() {
	Cube222 cube;
	std::cout << "2x2x2 Cube:" << std::endl;
	cube.applyRotation(R);
	cube.applyRotation(U);
	cube.printCube(true);
	return 0;
};