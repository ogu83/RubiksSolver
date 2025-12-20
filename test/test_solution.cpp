#include "RubiksSolver.h"
#include <iostream>

using namespace std;

int main() {
    Cube222 cube;

    // Set up the scrambled cube from README
    cube.setColor(TOP, {YELLOW, YELLOW, YELLOW, YELLOW});
    cube.setColor(FRONT, {RED, ORANGE, ORANGE, ORANGE});
    cube.setColor(RIGHT, {BLUE, GREEN, BLUE, BLUE});
    cube.setColor(BACK, {ORANGE, RED, RED, RED});
    cube.setColor(BOTTOM, {WHITE, WHITE, WHITE, WHITE});
    cube.setColor(LEFT, {GREEN, BLUE, GREEN, GREEN});

    cout << "Initial state:" << endl;
    cube.printState();
    cout << "isSolved(): " << (cube.isSolved() ? "YES" : "NO") << endl;
    cout << endl;

    // Apply the solution found by OptMT: D U RI FI R B R F
    cout << "Applying: D U RI FI R B R F" << endl;
    cube.applyRotation(D);
    cube.applyRotation(U);
    cube.applyRotation(RI);
    cube.applyRotation(FI);
    cube.applyRotation(R);
    cube.applyRotation(B);
    cube.applyRotation(R);
    cube.applyRotation(F);

    cout << "After applying solution:" << endl;
    cube.printState();
    cout << "isSolved(): " << (cube.isSolved() ? "YES" : "NO") << endl;
    cout << endl;

    // Reset and try the known solution: F UI B LI B R F
    cube.reset();
    cube.setColor(TOP, {YELLOW, YELLOW, YELLOW, YELLOW});
    cube.setColor(FRONT, {RED, ORANGE, ORANGE, ORANGE});
    cube.setColor(RIGHT, {BLUE, GREEN, BLUE, BLUE});
    cube.setColor(BACK, {ORANGE, RED, RED, RED});
    cube.setColor(BOTTOM, {WHITE, WHITE, WHITE, WHITE});
    cube.setColor(LEFT, {GREEN, BLUE, GREEN, GREEN});

    cout << "Applying known solution: F UI B LI B R F" << endl;
    cube.applyRotation(F);
    cube.applyRotation(UI);
    cube.applyRotation(B);
    cube.applyRotation(LI);
    cube.applyRotation(B);
    cube.applyRotation(R);
    cube.applyRotation(F);

    cout << "After applying known solution:" << endl;
    cube.printState();
    cout << "isSolved(): " << (cube.isSolved() ? "YES" : "NO") << endl;

    return 0;
}
