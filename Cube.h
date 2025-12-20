#ifndef CUBE_H
#define CUBE_H

#include <vector>
#include <string>
#include <map>

// Enums
enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE };
enum Rotation { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI, ROTATION_NONE };

// Global mappings
extern std::map<char, Color> charToColor;
extern std::map<std::string, Faces> tagToFace;
extern std::map<Rotation, Rotation> inverseRotation;

// Base Cube class
class Cube {
public:
	Cube(Color initialColor, int cRow, int cCol, int cFace);
	virtual ~Cube() = default;

	// Virtual methods to be implemented by derived classes
	virtual Cube* copy() const = 0;
	virtual void applyRotation(Rotation r) = 0;

	// Common methods
	void setColor(Faces face, Color color);
	void setColor(Faces face, const std::vector<Color>& colors);
	void setColor(Faces face, int row, int col, Color color);
	Color getColor(Faces face, int row, int col) const;

	void reset();
	void saveInitState();
	bool isSolved() const;

	void printState() const;
	std::string rotationsToString() const;

	// Helper methods for string conversion
	static std::string colorToString(Color color);
	static std::string faceToString(Faces face);
	static std::string rotationToString(Rotation r);

protected:
	int _cRow;
	int _cCol;
	int _cFace;
	std::vector<std::vector<std::vector<Color>>> _matrix;
	std::vector<std::vector<std::vector<Color>>> _initMatrix;
	std::vector<Rotation> _rotations;

	// Virtual methods for derived classes
	virtual void rotateFace(Faces face, bool clockwise) = 0;
	virtual void applyRotationInternal(Rotation r) = 0;

	void undoRotation(Rotation r);
	void applySolution(const std::vector<Rotation>& solution);
};

#endif // CUBE_H
