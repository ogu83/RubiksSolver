# RubiksSolver

## Compile

```bash
g++ -o RubiksSolver RubiksSolver.cpp
```

## Insight

```C
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

## Cube 3x3x3
Comming soon
