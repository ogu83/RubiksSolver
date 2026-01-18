# Build Instructions

## Compilation Commands

All executables are output to the `bin/` directory.

### Refactored Solvers (Using Cube222.h/cpp)

```bash
# Simple skeleton (no solver logic yet)
g++ -o bin/RubiksSolver.exe Cube.cpp Cube222.cpp RubiksSolver.cpp -std=c++20

# Multi-threaded DFS solver
g++ -o bin/RubiksSolverMT.exe Cube.cpp Cube222.cpp RubiksSolverMT.cpp -std=c++20 -pthread

# Single-threaded IDA* with heuristic
g++ -o bin/RubiksSolverOptimized.exe Cube.cpp Cube222.cpp RubiksSolverOptimized.cpp -std=c++20

# Multi-threaded IDA* with heuristic
g++ -o bin/RubiksSolverOptMT.exe Cube.cpp Cube222.cpp RubiksSolverOptMT.cpp -std=c++20 -pthread
```

### Test Programs

```bash
# Manual rotation test
g++ -o test/test_manual_rotation.exe test/test_manual_rotation.cpp -std=c++20

# Performance test
g++ -o test/test_mt_performance.exe test/test_mt_performance.cpp -std=c++20 -pthread

# Solution verification
g++ -o bin/verify_solution.exe verify_solution.cpp -std=c++20
```

## Usage Examples

### Solved Cube
```bash
./bin/RubiksSolverOptMT.exe
# Output: Already solved!
```

### Scrambled Cube (via command-line color specification)
```bash
./bin/RubiksSolverOptMT.exe -ft YYYY -ff ROOO -fr BGBB -fbk ORRR -fb WWWW -fl GBGG
```

**Note**: Setting colors via command line can create invalid/unsolvable cube states. For testing, it's better to scramble a solved cube using actual rotation moves.

## Compiler Requirements

- C++20 or later
- Supports: g++, clang++, MSVC
- Multi-threaded versions require `-pthread` flag

## Build All

```bash
# Build all solvers
g++ -o bin/RubiksSolver.exe Cube.cpp Cube222.cpp RubiksSolver.cpp -std=c++20
g++ -o bin/RubiksSolverMT.exe Cube.cpp Cube222.cpp RubiksSolverMT.cpp -std=c++20 -pthread
g++ -o bin/RubiksSolverOptimized.exe Cube.cpp Cube222.cpp RubiksSolverOptimized.cpp -std=c++20
g++ -o bin/RubiksSolverOptMT.exe Cube.cpp Cube222.cpp RubiksSolverOptMT.cpp -std=c++20 -pthread
```

## Clean

```bash
rm bin/*.exe
rm test/*.exe
```

## Folder Structure

```
RubiksSolver/
├── bin/               # Compiled executables
├── doc/               # Documentation (.md files)
├── test/              # Test programs and executables
├── Cube.h/cpp         # Base Cube class
├── Cube222.h/cpp      # 2x2x2 Cube implementation
├── RubiksSolver*.cpp  # Solver implementations
└── README.md          # Main documentation
```
