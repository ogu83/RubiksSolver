#include "RubiksSolver.h"
#include <iostream>

using namespace std;

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE };
enum Rotation { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };

// Simple cube to verify solution
class TestCube {
public:
    std::vector<std::vector<std::vector<Color>>> _matrix;
    int _cRow = 2, _cCol = 2, _cFace = 6;

    TestCube() : _matrix(6, std::vector<std::vector<Color>>(2, std::vector<Color>(2, WHITE))) {
        setColorsToInitState();
    }

    void setColorsToInitState() {
        setColor(FRONT, BLUE);
        setColor(RIGHT, RED);
        setColor(TOP, YELLOW);
        setColor(BOTTOM, WHITE);
        setColor(BACK, GREEN);
        setColor(LEFT, ORANGE);
    }

    void setColor(Faces face, Color color) {
        for (int r = 0; r < _cRow; r++) {
            for (int c = 0; c < _cCol; c++) {
                _matrix[face][r][c] = color;
            }
        }
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

    bool isSolved() const {
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

    void printState(const string& label) {
        cout << label << endl;
        const char* faceNames[] = {"TOP", "FRONT", "RIGHT", "BOTTOM", "BACK", "LEFT"};
        const char* colorNames[] = {"RED", "BLUE", "ORANGE", "GREEN", "WHITE", "YELLOW"};

        for (int f = 0; f < _cFace; ++f) {
            cout << faceNames[f] << ": ";
            for (int r = 0; r < _cRow; r++) {
                for (int c = 0; c < _cCol; c++) {
                    cout << colorNames[_matrix[f][r][c]][0];
                }
                if (r == 0) cout << "/";
            }
            cout << "  ";
        }
        cout << endl;
        cout << "Solved: " << (isSolved() ? "YES" : "NO") << endl;
        cout << endl;
    }

    void rotateFace(Faces face, bool clockwise) {
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

    void applyRotation(Rotation r) {
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

int main() {
    TestCube cube;

    // Set up the scrambled cube from README
    cout << "=== Verifying Solution: F UI B LI B R F ===" << endl;
    cout << endl;

    // TOP face: YYYY
    std::vector<Color> topFace = {YELLOW, YELLOW, YELLOW, YELLOW};
    cube.setColor(TOP, topFace);

    // FRONT face: ROOO
    std::vector<Color> frontFace = {RED, ORANGE, ORANGE, ORANGE};
    cube.setColor(FRONT, frontFace);

    // RIGHT face: BGBB
    std::vector<Color> rightFace = {BLUE, GREEN, BLUE, BLUE};
    cube.setColor(RIGHT, rightFace);

    // BACK face: ORRR
    std::vector<Color> backFace = {ORANGE, RED, RED, RED};
    cube.setColor(BACK, backFace);

    // BOTTOM face: WWWW
    std::vector<Color> bottomFace = {WHITE, WHITE, WHITE, WHITE};
    cube.setColor(BOTTOM, bottomFace);

    // LEFT face: GBGG
    std::vector<Color> leftFace = {GREEN, BLUE, GREEN, GREEN};
    cube.setColor(LEFT, leftFace);

    cube.printState("Initial state:");

    // Apply known solution: F UI B LI B R F
    cout << "Applying: F" << endl;
    cube.applyRotation(F);
    cube.printState("After F:");

    cout << "Applying: UI" << endl;
    cube.applyRotation(UI);
    cube.printState("After UI:");

    cout << "Applying: B" << endl;
    cube.applyRotation(B);
    cube.printState("After B:");

    cout << "Applying: LI" << endl;
    cube.applyRotation(LI);
    cube.printState("After LI:");

    cout << "Applying: B" << endl;
    cube.applyRotation(B);
    cube.printState("After B:");

    cout << "Applying: R" << endl;
    cube.applyRotation(R);
    cube.printState("After R:");

    cout << "Applying: F" << endl;
    cube.applyRotation(F);
    cube.printState("After F (Final):");

    if (cube.isSolved()) {
        cout << "SUCCESS: Solution verified!" << endl;
    } else {
        cout << "FAIL: Solution does not solve the cube!" << endl;
    }

    return 0;
}
