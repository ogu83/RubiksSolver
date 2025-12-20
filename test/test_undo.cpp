#include "RubiksSolverOptimized.cpp"
#include <iostream>

int main() {
    Cube222Opt cube;

    // Set up a specific scrambled state
    cube.setColor(TOP, {YELLOW, YELLOW, YELLOW, YELLOW});
    cube.setColor(FRONT, {RED, ORANGE, ORANGE, ORANGE});
    cube.setColor(RIGHT, {BLUE, GREEN, BLUE, BLUE});

    cube.saveInitState();

    std::cout << "Initial state:" << std::endl;
    cube.printState();

    // Apply F
    std::cout << "\nApplying F:" << std::endl;
    cube.applyRotation(F);
    cube.printState();

    // Undo F
    std::cout << "\nUndoing F:" << std::endl;
    cube.undoRotation(F);
    cube.printState();

    std::cout << "\nShould match initial state!" << std::endl;

    return 0;
}
