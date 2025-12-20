#include "Cube222.h"

// Constructor
Cube222::Cube222(Color initialColor, int cRow, int cCol, int cFace)
	: Cube(initialColor, cRow, cCol, cFace) {
}

// Copy
Cube* Cube222::copy() const {
	Cube222* newCube = new Cube222(*this);
	newCube->_matrix = this->_matrix;
	return newCube;
}

// Rotate a face
void Cube222::rotateFace(Faces face, bool clockwise) {
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

// Apply rotation (implementation from working RubiksSolver.cpp)
void Cube222::applyRotation(Rotation r) {
	applyRotationInternal(r);
	_rotations.push_back(r);
}

// Internal rotation logic
void Cube222::applyRotationInternal(Rotation r) {
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
