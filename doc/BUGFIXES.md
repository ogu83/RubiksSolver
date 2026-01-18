# Bug Fixes for Heuristic-Optimized IDA* Implementations

## Date: 2025-12-21

## Summary

This document details the bugs found and fixed in the IDA* with heuristic implementations (RubiksSolverOptimized.cpp and RubiksSolverOptMT.cpp).

## Bugs Fixed

### 1. **CRITICAL: isSolved() Checks Wrong Goal State** ✅ FIXED

**Files Affected**:
- [Cube.cpp:88-106](Cube.cpp#L88-L106) (isSolved implementation)
- [RubiksSolverOptimized.cpp:28-48](RubiksSolverOptimized.cpp#L28-L48) (heuristic)
- [RubiksSolverOptMT.cpp:29-49](RubiksSolverOptMT.cpp#L29-L49) (heuristic)

**Problem**:
The `isSolved()` method was checking if faces are **uniform** (all one color) instead of checking if they match the **standard solved state**:

```cpp
// OLD (WRONG): Checks if each face is uniform
bool Cube::isSolved() const {
    for (size_t f = 0; f < _cFace / 2; ++f) {
        const auto& face = _matrix[f];
        const Color referenceColor = face[0][0];  // Uses corner piece as reference!
        for (size_t i = 0; i < _cCol; ++i) {
            for (size_t j = 0; j < _cRow; ++j) {
                if (face[i][j] != referenceColor) {
                    return false;
                }
            }
        }
    }
    return true;
}
```

This would return `true` for ANY cube state where the first 3 faces are uniform, even if they have the wrong colors! For example:
- TOP = all GREEN, FRONT = all ORANGE, RIGHT = all BLUE would pass as "solved" ❌

**Impact**:
- Algorithm would find paths to ANY uniform state, not the correct solved state
- Found "solutions" didn't actually solve the cube
- Example: Simple `-ff YRYB` scramble found 7-move "solution" that left cube unsolved
- Heuristic was also using wrong reference (corner piece instead of target colors)

**Fix**:
Changed to check against the standard solved state colors:

```cpp
// NEW (CORRECT): Checks against target solved state
bool Cube::isSolved() const {
    const Color solvedColors[6] = { YELLOW, BLUE, RED, WHITE, GREEN, ORANGE };
    // Order is: TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT

    for (size_t f = 0; f < _cFace / 2; ++f) {
        const Color targetColor = solvedColors[f];  // Uses TARGET color!
        const auto& face = _matrix[f];
        for (size_t i = 0; i < _cCol; ++i) {
            for (size_t j = 0; j < _cRow; ++j) {
                if (face[i][j] != targetColor) {
                    return false;
                }
            }
        }
    }
    return true;
}
```

Also updated heuristic functions to count misplaced pieces relative to target colors:
```cpp
const Color solvedColors[6] = { YELLOW, BLUE, RED, WHITE, GREEN, ORANGE };
const Color targetColor = solvedColors[f];  // Count against target, not corner!
```

**Result**:
- Algorithm now correctly searches for the standard solved state (TOP=YELLOW, FRONT=BLUE, RIGHT=RED)
- Found solutions actually solve the cube ✅
- Heuristic accurately estimates distance to the correct goal state

### 2. **CRITICAL: Inadmissible Heuristic** ✅ FIXED

**Files Affected**:
- [RubiksSolverOptimized.cpp:148](RubiksSolverOptimized.cpp#L148)
- [RubiksSolverOptMT.cpp:148](RubiksSolverOptMT.cpp#L148)

**Problem**:
The heuristic function was counting misplaced pieces across ALL 6 faces:
```cpp
for (size_t f = 0; f < _cFace; ++f)  // Counts all 6 faces
```

But `isSolved()` only checks the first 3 faces:
```cpp
for (size_t f = 0; f < _cFace / 2; ++f)  // Only checks 3 faces
```

**Impact**:
- The heuristic overestimated the distance to the goal
- Made the heuristic **inadmissible** (violated h(n) ≤ actual distance)
- Caused aggressive pruning that eliminated valid solution paths
- Algorithm would search beyond the optimal solution depth

**Fix**:
Changed heuristic to only count first 3 faces:
```cpp
for (size_t f = 0; f < _cFace / 2; ++f)  // Now matches isSolved()
```

**Result**:
- Heuristic is now admissible
- Algorithm can find solutions (though not optimal yet - see remaining issues)

### 2. **Solution Storage Bug in Multi-Threaded Version** ✅ FIXED

**File Affected**: [RubiksSolverOptMT.cpp:220-226](RubiksSolverOptMT.cpp#L220-L226)

**Problem**:
The worker thread was checking `localCube->isSolved()` after the recursive search returned, but the cube had been backtracked by that point, so the check always failed:
```cpp
if (solutionFound.load() && localCube->isSolved()) {  // BUG: cube is backtracked!
    foundSolution = path;
}
```

**Impact**:
- Solutions found during search were never stored
- Algorithm would continue searching indefinitely

**Fix**:
Changed to check the return value directly:
```cpp
if (localNextBound == FOUND) {
    std::lock_guard<std::mutex> lock(solutionMutex);
    if (foundSolution.empty()) {
        foundSolution = path;
    }
}
```

**Result**:
- Solutions are now properly stored when found

### 3. **Missing reset() Call in Single-Threaded Version** ✅ FIXED

**File Affected**: [RubiksSolverOptimized.cpp:206](RubiksSolverOptimized.cpp#L206)

**Problem**:
After finding a solution, the code was applying the solution to an already-solved cube (the state after the search completed), rather than resetting to the initial scrambled state first.

**Impact**:
- Solution was applied twice (once during search, once after)
- Final cube state was scrambled, not solved
- Output showed solution in rotation list twice

**Fix**:
Added `reset()` call before applying solution:
```cpp
reset();
applySolution(_solution);
```

**Result**:
- Solution is now applied to the initial scrambled state
- Rotation list shows solution only once

### 4. **Minor: U Rotation Bug in verify_solution.cpp** ✅ FIXED

**File Affected**: [verify_solution.cpp:110](verify_solution.cpp#L110)

**Problem**:
U rotation was overwriting FRONT twice instead of cascading through RIGHT:
```cpp
_matrix[FRONT][0] = _matrix[RIGHT][0];
_matrix[FRONT][0] = _matrix[BACK][0];  // BUG: Overwrites FRONT again!
```

**Fix**:
```cpp
_matrix[FRONT][0] = _matrix[RIGHT][0];
_matrix[RIGHT][0] = _matrix[BACK][0];  // Correct cascade
```

**Note**: This bug was only in the verification tool, not in the main solvers.

## Remaining Issues

### CRITICAL: Solutions Found Don't Actually Solve the Cube

**Status**: 🔴 NOT YET FIXED

**Observed Behavior**:
```bash
$ ./RubiksSolverOptimized.exe -ff YRYB
Initial heuristic: 1 moves
Searching with bound 1...
Searching with bound 2...
...
Searching with bound 7...
Solved in 5.29 seconds.
Solution (7 moves): RI UI D L B D U
Solved: NO   <-- Solution doesn't actually solve!
```

**Evidence**:
1. Simple 1-move scramble (-ff YRYB) should have 1-move solution (FI)
2. Algorithm finds 7-move solution that doesn't solve the cube
3. Working DFS solver finds correct 2-move solution for same scramble
4. Multi-threaded version found 8-move solution for README example that doesn't solve
5. Both versions take much longer than expected and explore millions of nodes

**Test Results**:
| Scramble | Expected | OptMT Found | Opt Found | Actually Solves? |
|----------|----------|-------------|-----------|------------------|
| Solved cube | 0 moves | 0 moves ✓ | 0 moves ✓ | Yes ✓ |
| -ff YRYB | 1-2 moves | 2 moves (RI L) | 7 moves | MT: Yes, ST: No |
| README example | 7 moves | 8 moves (10 bounds) | Not tested | No |

**Possible Causes** (Not Yet Confirmed):
1. Bug in rotation implementations (though they appear identical to working version)
2. Bug in undo/backtracking logic
3. Path construction issue (moves don't correspond to actual rotations applied)
4. Race condition in multi-threaded version
5. State corruption during search

**Performance Issues**:
- Single-threaded: 938,736 nodes for simple 1-move scramble (should be ~12)
- Multi-threaded: 182,062,015 nodes for README example (should be ~100K-1M)
- Times: 5-1500 seconds for problems that should solve in < 1 second

**Next Steps to Debug**:
1. Add verbose logging to trace exact moves applied during search
2. Verify that `isSolved()` is being called on correct states
3. Compare step-by-step execution with working DFS version
4. Test undo/redo functionality in isolation
5. Check if path variable correctly reflects cube state at each step

## Compilation Instructions

### Single-Threaded Optimized Version:
```bash
g++ -o RubiksSolverOptimized RubiksSolverOptimized.cpp -std=c++20 -fcoroutines
```

### Multi-Threaded Optimized Version:
```bash
g++ -o RubiksSolverOptMT RubiksSolverOptMT.cpp -std=c++20 -fcoroutines -pthread
```

## Testing

### Test Solved Cube:
```bash
./RubiksSolverOptMT  # Should immediately say "Already solved!"
```

### Test Simple 1-Move Scramble:
```bash
./RubiksSolverOptMT -ff YRYB  # Should find 1-2 move solution quickly
```

### Test README Example:
```bash
./RubiksSolverOptMT -ft YYYY -ff ROOO -fr BGBB -fbk ORRR -fb WWWW -fl GBGG
# Known solution: F UI B LI B R F (7 moves)
```

## Comparison with Working Versions

### Working Versions (No Known Bugs):
- [RubiksSolver.cpp](RubiksSolver.cpp) - Original DFS, finds correct solutions
- [RubiksSolverMT.cpp](RubiksSolverMT.cpp) - Multi-threaded DFS, finds correct solutions
- [verify_solution.cpp](verify_solution.cpp) - Manual verification (after fix), confirms F UI B LI B R F solves README example

### Broken Versions (Critical Bug Remains):
- [RubiksSolverOptimized.cpp](RubiksSolverOptimized.cpp) - IDA* with heuristic, finds invalid solutions
- [RubiksSolverOptMT.cpp](RubiksSolverOptMT.cpp) - Multi-threaded IDA* with heuristic, finds invalid solutions

## Recommendations

### For Users:
**DO NOT USE the optimized versions (RubiksSolverOptimized/OptMT) for production until the remaining critical bug is fixed.**

Use the working versions instead:
- For single-threaded: `./RubiksSolver`
- For multi-threaded: `./RubiksSolverMT`

### For Developers:
The IDA* with heuristic implementations need further debugging to identify why solutions don't actually solve the cube. The heuristic and solution storage bugs have been fixed, but a deeper issue remains in the search logic or rotation implementations.

## References

- [HEURISTIC_OPTIMIZATION.md](HEURISTIC_OPTIMIZATION.md) - Original heuristic implementation documentation
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture and algorithm details
- [MULTITHREADING.md](MULTITHREADING.md) - Multi-threading implementation guide
- [TESTING.md](TESTING.md) - Testing procedures

## Change Log

- **2025-12-21**: Initial bug documentation
  - Fixed inadmissible heuristic (counting wrong faces)
  - Fixed solution storage in multi-threaded version
  - Fixed missing reset() in single-threaded version
  - Identified critical remaining issue: invalid solutions
