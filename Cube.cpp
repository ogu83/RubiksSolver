#include "Cube.h"
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;

// Global mappings
std::map<char, Color> charToColor = {
	{'R', RED}, {'B', BLUE}, {'O', ORANGE},
	{'G', GREEN}, {'W', WHITE}, {'Y', YELLOW}
};

std::map<std::string, Faces> tagToFace = {
	{"-ft", TOP}, {"-ff", FRONT}, {"-fr", RIGHT},
	{"-fb", BOTTOM}, {"-fbk", BACK}, {"-fl", LEFT}
};

std::map<Rotation, Rotation> inverseRotation = {
	{U, UI}, {UI, U}, {D, DI}, {DI, D},
	{R, RI}, {RI, R}, {L, LI}, {LI, L},
	{F, FI}, {FI, F}, {B, BI}, {BI, B},
	{ROTATION_NONE, ROTATION_NONE}
};

// Constructor
Cube::Cube(Color initialColor, int cRow, int cCol, int cFace)
	: _cRow(cRow), _cCol(cCol), _cFace(cFace),
	_matrix(cFace, std::vector<std::vector<Color>>(cCol, std::vector<Color>(cRow, initialColor))) {
	// Initialize with default colors
	setColor(TOP, YELLOW);
	setColor(FRONT, BLUE);
	setColor(RIGHT, RED);
	setColor(BOTTOM, WHITE);
	setColor(BACK, GREEN);
	setColor(LEFT, ORANGE);
	_rotations.clear();
}

// Set entire face to one color
void Cube::setColor(Faces face, Color color) {
	for (int r = 0; r < _cRow; r++) {
		for (int c = 0; c < _cCol; c++) {
			_matrix[face][r][c] = color;
		}
	}
}

// Set face from vector of colors
void Cube::setColor(Faces face, const std::vector<Color>& colors) {
	for (int i = 0; i < _cRow; ++i) {
		for (int j = 0; j < _cCol; ++j) {
			int idx = i * _cCol + j;
			if (idx < colors.size()) {
				_matrix[face][i][j] = colors[idx];
			}
		}
	}
}

// Set individual color
void Cube::setColor(Faces face, int row, int col, Color color) {
	if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
		_matrix[face][row][col] = color;
	}
}

// Get color
Color Cube::getColor(Faces face, int row, int col) const {
	if (row >= 0 && row < _cRow && col >= 0 && col < _cCol) {
		return _matrix[face][row][col];
	}
	return Color::WHITE;
}

// Save initial state
void Cube::saveInitState() {
	_initMatrix = _matrix;
}

// Reset to initial state
void Cube::reset() {
	_matrix = _initMatrix;
	_rotations.clear();
}

// Check if solved (matches standard solved state)
bool Cube::isSolved() const {
	// Define the standard solved state colors for each face
	const Color solvedColors[6] = { YELLOW, BLUE, RED, WHITE, GREEN, ORANGE };
	// Order is: TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT

	// Check first 3 faces match the solved state
	for (size_t f = 0; f < _cFace / 2; ++f) {
		const Color targetColor = solvedColors[f];
		const auto& face = _matrix[f];
		for (size_t i = 0; i < _cCol; ++i) {
			for (size_t j = 0; j < _cRow; ++j) {
				if (face[i][j] != targetColor) {
					return false;
				}
			}
		}
	}
	return true;
}

// Undo rotation
void Cube::undoRotation(Rotation r) {
	applyRotationInternal(inverseRotation[r]);
	if (!_rotations.empty()) {
		_rotations.pop_back();
	}
}

// Apply solution
void Cube::applySolution(const std::vector<Rotation>& solution) {
	for (Rotation move : solution) {
		applyRotation(move);
	}
}

// Print current state
void Cube::printState() const {
	std::string solvedStr = isSolved() ? "YES" : "NO";
	std::cout << "Solved: " << solvedStr << std::endl;
	std::cout << "Rotations: " << rotationsToString() << std::endl;

	for (int f = 0; f < _cFace / 2; ++f) {
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

// Get rotations as string
std::string Cube::rotationsToString() const {
	std::stringstream ss;
	for (size_t i = 0; i < _rotations.size(); ++i) {
		ss << rotationToString(_rotations[i]);
		if (i < _rotations.size() - 1) ss << " ";
	}
	return ss.str();
}

// Static helper methods
std::string Cube::colorToString(Color color) {
	switch (color) {
	case RED: return "RED";
	case BLUE: return "BLUE";
	case ORANGE: return "ORANGE";
	case GREEN: return "GREEN";
	case WHITE: return "WHITE";
	case YELLOW: return "YELLOW";
	default: return "UNDEFINED";
	}
}

std::string Cube::faceToString(Faces face) {
	switch (face) {
	case TOP: return "TOP";
	case FRONT: return "FRONT";
	case RIGHT: return "RIGHT";
	case BOTTOM: return "BOTTOM";
	case BACK: return "BACK";
	case LEFT: return "LEFT";
	default: return "NONE";
	}
}

std::string Cube::rotationToString(Rotation r) {
	switch (r) {
	case U: return "U";
	case D: return "D";
	case R: return "R";
	case L: return "L";
	case F: return "F";
	case B: return "B";
	case UI: return "UI";
	case DI: return "DI";
	case RI: return "RI";
	case LI: return "LI";
	case FI: return "FI";
	case BI: return "BI";
	default: return "NONE";
	}
}
