#include <iostream>
#include <vector>

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE };

int main() {
    std::vector<std::vector<std::vector<Color>>> matrix(6, std::vector<std::vector<Color>>(2, std::vector<Color>(2)));
    
    // Set up solved state
    for(int r=0; r<2; r++) for(int c=0; c<2; c++) {
        matrix[TOP][r][c] = YELLOW;
        matrix[FRONT][r][c] = BLUE;
        matrix[RIGHT][r][c] = RED;
        matrix[BOTTOM][r][c] = WHITE;
        matrix[BACK][r][c] = GREEN;
        matrix[LEFT][r][c] = ORANGE;
    }
    
    // Now set FRONT face to YRYB (which is what -ff YRYB does)
    matrix[FRONT][0][0] = YELLOW;
    matrix[FRONT][0][1] = RED;
    matrix[FRONT][1][0] = YELLOW;
    matrix[FRONT][1][1] = BLUE;
    
    std::cout << "Initial FRONT face:" << std::endl;
    std::cout << matrix[FRONT][0][0] << " " << matrix[FRONT][0][1] << std::endl;
    std::cout << matrix[FRONT][1][0] << " " << matrix[FRONT][1][1] << std::endl;
    
    // Apply FI (counter-clockwise) to FRONT face itself
    Color temp = matrix[FRONT][0][0];
    matrix[FRONT][0][0] = matrix[FRONT][0][1];
    matrix[FRONT][0][1] = matrix[FRONT][1][1];
    matrix[FRONT][1][1] = matrix[FRONT][1][0];
    matrix[FRONT][1][0] = temp;
    
    std::cout << "\nAfter FI rotation of FRONT face:" << std::endl;
    std::cout << matrix[FRONT][0][0] << " " << matrix[FRONT][0][1] << std::endl;
    std::cout << matrix[FRONT][1][0] << " " << matrix[FRONT][1][1] << std::endl;
    
    std::cout << "\nExpected: 1 3/5 1 (RED BLUE / YELLOW YELLOW)" << std::endl;
    std::cout << "That would make FRONT partially uniform..." << std::endl;
    
    return 0;
}
