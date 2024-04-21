# Rubik's Cube Solver

This is a rubik's cube solver using deep first search algoritms in c++.

## Compile

```bash
g++ -o RubiksSolver RubiksSolver.cpp
```

## Insight

```cpp
enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE };

std::map<char, Color> charToColor = {
    {'R', RED}, {'B', BLUE}, {'O', ORANGE}, {'G', GREEN}, {'W', WHITE}, {'Y', YELLOW}
};

std::map<std::string, Faces> tagToFace = {
    {"-ft", TOP}, {"-ff", FRONT}, {"-fr", RIGHT}, {"-fb", BOTTOM}, {"-fbk", BACK}, {"-fl", LEFT}
};
```

## Cube 2x2x2
This is a Mini Rubik's Cube Solver

![alt text](https://target.scene7.com/is/image/Target/GUEST_6e5b21e4-fa59-4eb2-8d18-75a226f31f28?wid=488&hei=488&fmt=pjpeg)


### How to run 
Each face is identified by a tag (like -ft for the top face), followed by a string of color initials (e.g., WRBG for White, Red, Blue, Green).
```bash
./RubiksSolver -ft WRBG -ff RYGB -fr GWBY -fbk OYGR -fb BORG -fl WYOB
```

This command sets each face of the cube with specified colors in a 2x2 layout. This approach provides a flexible and clear method for initializing the Rubik’s cube from command line arguments, reflecting a specific scrambled state or configuration for testing or demonstration purposes.

### Test Case
```powershell
PS C:\Users\oguz\source\repos\RubiksSolver\out\build\x64-release> .\RubiksSolver.exe -ft YYYY -ff ROOO -fr BGBB -fbk ORRR -fb WWWW -fl GBGG
2x2x2 Cube:
Solved: NO
Rotations:
Face: TOP
YELLOW YELLOW
YELLOW YELLOW

Face: FRONT
RED ORANGE
ORANGE ORANGE

Face: RIGHT
BLUE GREEN
BLUE BLUE

Face: BOTTOM
WHITE WHITE
WHITE WHITE

Face: BACK
ORANGE RED
RED RED

Face: LEFT
GREEN BLUE
GREEN GREEN

12 combinations testing.
0.0002065 seconds elapsed.
Increasing depth to 2. Continue search...
144 combinations testing.
Increasing depth to 3. Continue search...
1728 combinations testing.
0.0023806 seconds elapsed.
Increasing depth to 4. Continue search...
20736 combinations testing.
0.0109181 seconds elapsed.
Increasing depth to 5. Continue search...
248832 combinations testing.
0.0789769 seconds elapsed.
Increasing depth to 6. Continue search...
2985984 combinations testing.
1.01422 seconds elapsed.
Increasing depth to 7. Continue search...
35831808 combinations testing.
Solved in 14.7959 seconds.
Solution: F UI B LI B R F
Solved: YES
Rotations: F UI B LI B R F
Face: TOP
WHITE WHITE
WHITE WHITE

Face: FRONT
BLUE BLUE
BLUE BLUE

Face: RIGHT
ORANGE ORANGE
ORANGE ORANGE

Face: BOTTOM
YELLOW GREEN
YELLOW RED

Face: BACK
GREEN RED
RED YELLOW

Face: LEFT
YELLOW GREEN
RED GREEN
```

## Cube 3x3x3
Comming soon
