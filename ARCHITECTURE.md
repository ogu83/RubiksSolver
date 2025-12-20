# Rubik's Cube Solver - Architecture Document

## Overview

This project implements a Rubik's Cube solver using depth-first search (DFS) algorithms in C++. The current implementation supports 2x2x2 (Pocket Cube) solving with an optimized IDA* search algorithm.

## System Architecture

### Core Components

```
┌─────────────────────────────────────────────────────┐
│                   Main Program                       │
│            (Command-line Interface)                  │
└────────────────┬────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────┐
│              Cube (Base Class)                       │
│  - State Management                                  │
│  - Rotation Tracking                                 │
│  - Solver Algorithms (IDA*, DFS)                     │
│  - Move Pruning Logic                                │
└────────────────┬────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────┐
│            Cube222 (Derived Class)                   │
│  - 2x2x2 Specific Rotations                          │
│  - Face Rotation Implementation                      │
│  - State Application                                 │
└─────────────────────────────────────────────────────┘
```

## Class Hierarchy

### Cube (Base Class)

**Purpose**: Abstract base class providing core cube functionality and solver algorithms.

**Key Responsibilities**:
- State management (cube configuration storage)
- Rotation history tracking
- Solver algorithms (IDA*, legacy DFS)
- Move validation and pruning
- Solution verification

**Data Structures**:
```cpp
_matrix: vector<vector<vector<Color>>>  // 3D array: [face][row][col]
_initMatrix: vector<vector<vector<Color>>>  // Initial state for reset
_rotations: vector<Rotation>  // Move history
_solution: vector<Rotation>  // Found solution
```

**Key Methods**:
- `isSolved()`: Fast solution check (only checks 3 faces)
- `idaStar()`: Optimized iterative deepening search
- `isRedundantMove()`: Move pruning logic
- `applyRotation()`: Virtual method for move application

### Cube222 (Derived Class)

**Purpose**: Concrete implementation for 2x2x2 Rubik's Cube.

**Key Responsibilities**:
- Implementing rotation mechanics for all 12 moves (U, D, R, L, F, B + inverses)
- Face rotation (90° clockwise/counterclockwise)
- Edge and corner piece movement

**Implementation Details**:
- Each rotation affects 4 edges and rotates 1 face
- Inverse moves denoted with "I" suffix (UI, DI, etc.)
- Uses temporary buffers for safe piece swapping

## Algorithm Design

### IDA* Search (Primary Algorithm)

**Approach**: Iterative deepening depth-first search with move pruning.

**Key Features**:
1. **Iterative Deepening**: Searches depth 1, 2, 3... until solution found
2. **Move Pruning**: Eliminates redundant move sequences
3. **Backtracking**: Efficient state reversal using `undoRotation()`

**Pruning Rules**:
```cpp
1. No inverse immediately after move (U followed by UI cancels out)
2. No duplicate consecutive moves (U followed by U)
```

**Performance Characteristics**:
- Time Complexity: O(b^d) where b=12 moves, d=depth
- Space Complexity: O(d) due to recursion depth
- Typical 2x2x2 solution: 7-11 moves, ~10-15 seconds

**Algorithm Flow**:
```
For depth = 1 to MAX_DEPTH:
    For each state in depth limit:
        If solved: return solution
        For each possible move:
            If not redundant:
                Apply move
                Recurse (depth + 1)
                If solved: return
                Undo move (backtrack)
```

### Legacy DFS (Secondary Algorithm)

**Approach**: Brute-force combination generation and testing.

**Characteristics**:
- Pre-generates all move combinations at given depth
- Tests each combination sequentially
- Less efficient but simpler to understand
- Used as fallback/comparison algorithm

## Data Representation

### Color Encoding
```cpp
enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED }
```

### Face Naming Convention
```cpp
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE }
```

Standard color mapping (solved state):
- TOP: Yellow
- FRONT: Blue
- RIGHT: Red
- BOTTOM: White
- BACK: Green
- LEFT: Orange

### Move Notation
```cpp
U/UI   - Top face clockwise/counterclockwise
D/DI   - Bottom face clockwise/counterclockwise
R/RI   - Right face clockwise/counterclockwise
L/LI   - Left face clockwise/counterclockwise
F/FI   - Front face clockwise/counterclockwise
B/BI   - Back face clockwise/counterclockwise
```

## State Management

### State Storage
- 3D vector structure: `_matrix[face][row][col]`
- Each face is 2x2 for Cube222
- Total storage: 6 faces × 2 rows × 2 cols = 24 color values

### State Operations
1. **Save Initial State**: `saveInitState()` - Preserves starting configuration
2. **Reset**: `reset()` - Restores to initial state
3. **Clone**: `copy()` - Creates independent cube copy

### Solution Verification
```cpp
bool isSolved() {
    // Optimization: Only check 3 faces (opposite faces always match)
    for (face in [TOP, FRONT, RIGHT]) {
        if (not all cells same color) return false;
    }
    return true;
}
```

## Performance Optimizations

### Current Optimizations

1. **Move Pruning**
   - Eliminates inverse moves: `U` followed by `UI`
   - Eliminates duplicate moves: `U` followed by `U`
   - Reduces search space by ~40-50%

2. **Partial Solution Check**
   - Only checks 3 of 6 faces (optimization in `isSolved()`)
   - Saves ~50% comparison time

3. **In-place State Modification**
   - No state copying during search
   - Uses backtracking with `undoRotation()`

4. **Early Termination**
   - Stops immediately when solution found
   - No unnecessary exploration

### Performance Bottlenecks

1. **State Comparison**: `isSolved()` called millions of times
2. **Deep Recursion**: Stack overhead for depth > 10
3. **No Heuristic**: IDA* doesn't use heuristic (present but unused)
4. **No State Caching**: Repeated states explored multiple times

## Input/Output Interface

### Command-Line Interface
```bash
./RubiksSolver -ft YYYY -ff ROOO -fr BGBB -fbk ORRR -fb WWWW -fl GBGG
```

**Input Format**:
- Face tag: `-ft` (top), `-ff` (front), `-fr` (right), `-fb` (bottom), `-fbk` (back), `-fl` (left)
- Color string: 4 characters for 2x2 face (e.g., "YYYY")
- Color codes: R=Red, B=Blue, O=Orange, G=Green, W=White, Y=Yellow

**Output Format**:
```
Solved: NO/YES
Rotations: [move sequence]
Face: [face name]
[Color grid]
...
Solving statistics:
- Nodes explored
- Time taken
- Solution moves
```

## Error Handling

### Current Error Handling
1. Index bounds checking in `setColor()` and `getColor()`
2. Invalid face tag detection in main()
3. Undefined color handling

### Missing Error Handling
- No validation of cube solvability
- No detection of impossible configurations
- No timeout mechanism for long searches

## Compilation Requirements

### Standard Requirements
```bash
g++ -o RubiksSolver RubiksSolver.cpp
```

### C++20 Features
- Requires `-std=c++20 -fcoroutines` flag
- Uses modern C++ features (auto, range-based loops, lambdas)

## Design Patterns Used

1. **Template Method Pattern**: `Cube` base class with virtual methods
2. **Strategy Pattern**: Multiple solver algorithms (IDA*, legacy DFS)
3. **Command Pattern**: Rotation enumeration and application
4. **Memento Pattern**: State save/restore functionality

## Extensibility Points

### Adding New Cube Sizes
1. Create derived class (e.g., `Cube333`)
2. Override `applyRotationInternal()` and `rotateFace()`
3. Adjust `_cRow`, `_cCol` in constructor
4. Implement size-specific rotation logic

### Adding New Algorithms
1. Add new solver method in `Cube` class
2. Implement algorithm using existing primitives
3. Call from `main()` or expose via command-line flag

### Adding Heuristics
1. Enhance `heuristic()` method
2. Integrate into IDA* search
3. Use for move ordering or pruning

## Dependencies

### Standard Library
- `<iostream>`: I/O operations
- `<vector>`: Dynamic arrays
- `<map>`: Lookup tables
- `<string>`: String manipulation
- `<algorithm>`: STL algorithms
- `<chrono>`: Timing
- `<unordered_set>`: Removed in optimization

### External Dependencies
- None (standalone implementation)

## Code Quality Metrics

### Strengths
- Clear class hierarchy
- Well-organized code structure
- Comprehensive rotation implementation
- Multiple solver algorithms

### Areas for Improvement
- Limited comments/documentation in code
- Magic numbers (e.g., depth limits)
- Long methods (rotation logic could be refactored)
- No unit tests
- Minimal error handling

## Thread Safety

**Current Status**: Not thread-safe
- Shared state in `Cube` class
- No synchronization mechanisms
- Single-threaded execution

**Implications**: Cannot parallelize search without modifications

## Memory Management

- Stack-based allocation for main cube
- Heap allocation in `copy()` method (not currently used in main path)
- No memory leaks in current implementation
- Automatic cleanup via RAII

## Configuration

### Hardcoded Parameters
- Maximum search depth: 14 (line 196)
- Heuristic divisor: 8 (line 157)
- Move set: 12 moves (U, D, R, L, F, B + inverses)

### Potential Configuration Points
- Search depth limit
- Timeout duration
- Output verbosity
- Algorithm selection
- Heuristic weights
