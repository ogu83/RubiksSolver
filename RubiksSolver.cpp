#include "Cube222.h"
#include <iostream>
#include <chrono>
#include <algorithm>

using namespace std;

int main(int argc, char* argv[]) {
	Cube222 cube;

	// Parse command line arguments
	for (int i = 1; i < argc; i += 2) {
		if (i + 1 < argc) {
			std::string tag = argv[i];
			std::string values = argv[i + 1];
			std::vector<Color> colors;

			std::transform(values.begin(), values.end(), std::back_inserter(colors),
				[](char c) -> Color { return charToColor.count(c) > 0 ? charToColor[c] : UNDEFINED; });

			if (tagToFace.count(tag) > 0) {
				cube.setColor(tagToFace[tag], colors);
			}
			else {
				std::cout << "Invalid face tag: " << tag << std::endl;
			}
		}
	}

	cube.saveInitState();

	std::cout << "2x2x2 Cube:" << std::endl;
	cube.printState();

	// Note: DFS solver logic would go here
	// For now, this is just the skeleton

	std::cout << "\nNote: DFS solver not yet implemented in refactored version." << std::endl;
	std::cout << "Use original RubiksSolver.cpp for solving." << std::endl;

	return 0;
}
