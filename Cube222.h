#ifndef CUBE222_H
#define CUBE222_H

#include "Cube.h"

class Cube222 : public Cube {
public:
	Cube222(Color initialColor = Color::WHITE, int cRow = 2, int cCol = 2, int cFace = 6);

	Cube* copy() const override;
	void applyRotation(Rotation r) override;

protected:
	void rotateFace(Faces face, bool clockwise) override;
	void applyRotationInternal(Rotation r) override;
};

#endif // CUBE222_H
