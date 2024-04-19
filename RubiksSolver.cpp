// RubiksSolver.cpp : Defines the entry point for the application.
//

#include "RubiksSolver.h"
#include <vector>
#include <array>

using namespace std;

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW };
enum Faces { FRONT, RIGHT, BACK, LEFT, TOP, BOTTOM };

// Convert Color enum to string
std::string colorToString(Color color) {
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

// Convert Faces enum to string
std::string faceToString(Faces face) {
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

class Cube {
public:
    virtual ~Cube() {}

protected:

};

class Cube222 : public Cube {
public:
    Cube222(Color initialColor = Color::WHITE) {
        for (auto& face : _matrix) {
            for (auto& row : face) {
                row.fill(initialColor);
            }
        }
    }

    // Function to set the color of a specific cell
    void setColor(Faces face, int row, int col, Color color) {
        if (row >= 0 && row < 2 && col >= 0 && col < 2) {
            _matrix[face][row][col] = color;
        }
        else {
            std::cerr << "Index out of bounds error." << std::endl;
        }
    }

    // Function to get the color of a specific cell
    Color getColor(Faces face, int row, int col) const {
        if (row >= 0 && row < 2 && col >= 0 && col < 2) {
            return _matrix[face][row][col];
        }
        else {
            std::cerr << "Index out of bounds error." << std::endl;
            return Color::WHITE;  // Default return
        }
    }

    // Utility to print the cube's configuration
    void printCube() const {
        static const char* faceNames[] = { "FRONT", "RIGHT", "BACK", "LEFT", "TOP", "BOTTOM" };
        for (int f = 0; f < 6; ++f) {
            std::cout << faceNames[f] << " Face:" << std::endl;
            for (const auto& row : _matrix[f]) {
                for (Color color : row) {
                    std::cout << color << " ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }

protected:
private:
    /// <summary>
    /// An array of 6 faces, each face is a 2x2 matrix of Color enums
    /// </summary>
    std::array<std::array<std::array<Color, 2>, 2>, 6> _matrix;
};


int main()
{
    /*Color myColor = GREEN;
    switch (myColor) {
    case RED:
        cout << "Red\n";
        break;
    case BLUE:
        cout << "Blue\n";
        break;
    case ORANGE:
        cout << "Orange\n";
        break;
    case GREEN:
        cout << "Green\n";
        break;
    case WHITE:
        cout << "White\n";
        break;
    case YELLOW:
        cout << "Yellow\n";
        break;
    default:
        cout << "Unknown color\n";
    }

    cout << "Hello CMake." << endl;*/

    Cube222 cube;  // Initializes all to WHITE by default

    // Setting some colors explicitly
    cube.setColor(FRONT, 0, 0, RED);
    cube.setColor(FRONT, 0, 1, BLUE);
    cube.setColor(FRONT, 1, 0, GREEN);
    cube.setColor(FRONT, 1, 1, ORANGE);

    cube.setColor(RIGHT, 0, 0, YELLOW);
    cube.setColor(RIGHT, 0, 1, WHITE);
    cube.setColor(RIGHT, 1, 0, BLUE);
    cube.setColor(RIGHT, 1, 1, GREEN);

    // Printing the cube's configuration
    std::cout << "Mini Rubik's Cube Configuration:" << std::endl;
    cube.printCube();

	return 0;
}
