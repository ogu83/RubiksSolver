#include "RubiksSolver.h"
#include <iostream>

using namespace std;

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE };
enum Rotation { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };

std::map<char, Color> charToColor = {
{'R', RED}, {'B', BLUE}, {'O', ORANGE}, {'G', GREEN}, {'W', WHITE}, {'Y', YELLOW}
};

int main() {
    // Test: Create a solved cube, scramble with one move, then solve
    cout << "Test: One move scramble" << endl;
    
    // Cube2 22 cube;
    // Apply one move (U)
    // cube.applyRotation(U);
    // cube.saveInitState();
    // cube.dfs();
    
    return 0;
}
