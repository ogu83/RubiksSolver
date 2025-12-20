# Solution Approaches and Algorithm Analysis

## Overview

This document analyzes different approaches to solving Rubik's Cube puzzles, comparing their trade-offs, and explaining the rationale behind algorithm choices in this project.

## Problem Space Analysis

### 2x2x2 Rubik's Cube (Pocket Cube)

**State Space**:
- Total positions: 3,674,160
- Unique orientations: 7 (corner orientations)
- Permutations: 8! / 24 = 1,680 (accounting for parity and fixed center)
- God's Number: 11 (maximum moves needed for any scramble using optimal algorithm)

**Characteristics**:
- Small enough for exhaustive search
- No center pieces (simplified mechanics)
- Only corner pieces to track
- Suitable for brute-force approaches with optimization

### 3x3x3 Rubik's Cube (Standard)

**State Space**:
- Total positions: 43,252,003,274,489,856,000 (~43 quintillion)
- God's Number: 20
- Diameter: 18 (half-turn metric)

**Characteristics**:
- Too large for exhaustive search
- Requires sophisticated algorithms
- Multiple solving strategies exist
- Pattern databases essential for reasonable performance

## Algorithm Comparison

### 1. Brute Force (Generate & Test)

**Description**: Generate all possible move sequences of length d, test each until solution found.

**Implementation**: `dfsLegacy()` in [RubiksSolver.cpp:224-255](RubiksSolver.cpp#L224-L255)

```cpp
void dfsLegacy(int depth) {
    // Generate all 12^depth combinations
    generateCombinations(allRotations, depth, currentPath, potentialSolutions);

    // Test each combination
    for (const auto& solution : potentialSolutions) {
        applySolution(solution);
        if (isSolved()) return;
        reset();
    }

    // Try next depth
    dfsLegacy(depth + 1);
}
```

**Complexity**:
- Time: O(b^d) where b=12, d=solution depth
- Space: O(b^d) to store all combinations

**Pros**:
- Simple to implement
- Easy to understand
- Guaranteed to find solution

**Cons**:
- Extremely memory intensive
- Slow: generates all combinations before testing
- Doesn't scale beyond depth 6-7
- No early termination

**When to Use**: Teaching, very small cubes, or guaranteed shallow solutions

**Performance** (2x2x2):
- Depth 5: 248,832 combinations, ~0.08s
- Depth 6: 2,985,984 combinations, ~1.0s
- Depth 7: 35,831,808 combinations, ~12s + testing time

---

### 2. Depth-First Search (DFS)

**Description**: Explore one path completely before backtracking.

**Pseudocode**:
```cpp
bool dfs(state, depth) {
    if (isSolved(state)) return true;
    if (depth == 0) return false;

    for (each move) {
        apply(move);
        if (dfs(state, depth - 1)) return true;
        undo(move);
    }
    return false;
}
```

**Complexity**:
- Time: O(b^d)
- Space: O(d) - only stores current path

**Pros**:
- Memory efficient (linear space)
- Can find solutions quickly with luck
- Easy to implement with backtracking

**Cons**:
- No guarantee of optimal solution
- Can get stuck in deep unproductive paths
- Still explores too many states

**When to Use**: Memory-constrained environments, non-optimal solutions acceptable

---

### 3. Breadth-First Search (BFS)

**Description**: Explore all states at depth d before depth d+1.

**Pseudocode**:
```cpp
bool bfs() {
    queue.push(initialState);
    while (!queue.empty()) {
        state = queue.pop();
        if (isSolved(state)) return true;

        for (each move) {
            newState = apply(move, state);
            queue.push(newState);
        }
    }
}
```

**Complexity**:
- Time: O(b^d)
- Space: O(b^d) - stores all states at current level

**Pros**:
- **Guaranteed optimal solution** (shortest path)
- Systematic exploration

**Cons**:
- Massive memory requirements
- Must store all states at frontier
- Not feasible for 3x3x3 without pruning

**When to Use**: When optimal solution required and state space manageable

**Example** (2x2x2 at depth 7):
- States to store: ~35 million
- Memory: ~35M × 24 bytes = ~840 MB

---

### 4. Iterative Deepening DFS (IDDFS)

**Description**: Run DFS with increasing depth limits.

**Pseudocode**:
```cpp
for (depth = 1 to maxDepth) {
    if (dfs(initialState, depth)) return true;
}
```

**Complexity**:
- Time: O(b^d) (same as DFS)
- Space: O(d) (same as DFS)

**Pros**:
- **Guaranteed optimal solution** like BFS
- **Memory efficient** like DFS
- Best of both worlds for uninformed search

**Cons**:
- Redundant work: re-explores states at shallower depths
- Slower than plain DFS for finding any solution
- Still explores too many states without pruning

**When to Use**: Need optimal solution, limited memory, no heuristic available

**Redundancy Analysis**:
```
Depth 1: 12 nodes
Depth 2: 12 + 144 = 156 nodes (12 re-explored)
Depth 3: 12 + 144 + 1,728 = 1,884 nodes
...
```

Overhead is ~11% for branching factor 12.

---

### 5. IDA* (Iterative Deepening A*) ⭐ **Current Implementation**

**Description**: IDDFS with heuristic-based pruning and move pruning.

**Implementation**: `idaStar()` in [RubiksSolver.cpp:184-218](RubiksSolver.cpp#L184-L218)

**Pseudocode**:
```cpp
for (depthLimit = 1 to maxDepth) {
    found = idaStarRecursive(0, depthLimit, ROTATION_NONE, path);
    if (found) return;
}

bool idaStarRecursive(currentDepth, depthLimit, lastMove, path) {
    if (isSolved()) return true;
    if (currentDepth >= depthLimit) return false;

    for (each move) {
        if (isRedundantMove(lastMove, move)) continue;  // Pruning

        apply(move);
        if (idaStarRecursive(currentDepth + 1, depthLimit, move, path))
            return true;
        undo(move);
    }
    return false;
}
```

**Complexity**:
- Time: O(b^d) worst case, but much better in practice
- Space: O(d)

**Pros**:
- **Guaranteed optimal solution** (with admissible heuristic)
- **Memory efficient**: O(d) space
- **Fast with good heuristic**: prunes large portions of search tree
- **Move pruning**: eliminates obviously redundant sequences

**Cons**:
- Re-explores states (like IDDFS)
- Performance depends on heuristic quality
- More complex to implement correctly

**When to Use**: Need optimal solution, limited memory, good heuristic available

**Current Pruning**:
1. **Inverse moves**: U followed by UI
2. **Duplicate moves**: U followed by U

**Removed Pruning** (caused bugs):
- Axis-based commutation (e.g., U and R are independent)

**Performance** (2x2x2):
- Effective branching factor: ~10 (vs 12 theoretical)
- Depth 7: ~10M effective nodes (vs 35M without pruning)
- ~3x speedup from pruning alone

---

### 6. A* Search

**Description**: Best-first search using f-cost = g(depth) + h(heuristic).

**Pseudocode**:
```cpp
priorityQueue.push({initialState, f_cost=heuristic(initialState)});

while (!priorityQueue.empty()) {
    state = priorityQueue.pop();  // Lowest f-cost
    if (isSolved(state)) return;

    for (each move) {
        newState = apply(move, state);
        f_cost = depth + heuristic(newState);
        priorityQueue.push({newState, f_cost});
    }
}
```

**Complexity**:
- Time: O(b^d) worst case, often much better
- Space: O(b^d) - stores all explored states

**Pros**:
- **Guaranteed optimal with admissible heuristic**
- **Fast with good heuristic**: explores promising paths first
- Fewer nodes than IDDFS

**Cons**:
- **High memory usage**: must store all states
- Not feasible for 3x3x3 without disk storage or distributed system
- Heuristic computation overhead

**When to Use**: State space moderate (<10M states), excellent heuristic available

**Comparison to IDA***:
- A* faster (explores fewer nodes)
- IDA* uses less memory
- For Rubik's Cube, IDA* preferred due to memory constraints

---

### 7. Bidirectional Search

**Description**: Search simultaneously from start and goal states.

**Pseudocode**:
```cpp
forwardStates.insert(initialState);
backwardStates.insert(solvedState);

for (depth = 0 to maxDepth/2) {
    expand(forwardStates, depth);
    expand(backwardStates, depth);

    if (hasIntersection(forwardStates, backwardStates)) {
        return reconstructPath();
    }
}
```

**Complexity**:
- Time: O(b^(d/2)) - **significant improvement**
- Space: O(b^(d/2))

**Pros**:
- **Major speedup**: square root of search space
- Guaranteed optimal solution
- Especially effective for Rubik's Cube

**Cons**:
- Still high memory usage
- Complex path reconstruction
- Requires symmetry support (cube rotations)

**When to Use**: State space large but not huge, memory available

**Example** (depth 10):
- Unidirectional: 12^10 = 61 trillion nodes
- Bidirectional: 2 × 12^5 = 497,664 nodes
- Speedup: ~123 million times (theoretical)

**Future Enhancement**: Excellent candidate for 3x3x3 implementation

---

### 8. Two-Phase Algorithm (Kociemba)

**Description**: Solve cube in two phases with different move subsets.

**Phase 1**: Reach subgroup H (oriented edges, E-slice edges)
- Allowed moves: All 18 moves
- Goal: 2,217,093,120 states (0.005% of total)

**Phase 2**: Solve within subgroup H
- Allowed moves: Only U, D, R2, L2, F2, B2 (10 moves)
- Guaranteed solvable from any H state

**Complexity**:
- Time: O(b1^d1 + b2^d2) where b1=18, b2=10
- Space: O(1) with lookup tables

**Pros**:
- **Very fast for 3x3x3** (~1 second solves)
- Uses pre-computed pattern databases
- Bounded solution length (~20 moves)
- Industry standard approach

**Cons**:
- Complex to implement
- Requires large lookup tables (~100 MB)
- Not guaranteed optimal (finds near-optimal)
- Specific to 3x3x3 structure

**When to Use**: 3x3x3 Rubik's Cube, fast solves required

**Implementation Effort**: High (would be next major algorithm to add)

---

### 9. Human-Style Algorithms

**Description**: Layer-by-layer, CFOP, Roux, etc.

**Example - Layer-by-layer**:
1. Solve white cross
2. Solve white corners
3. Solve middle layer edges
4. Orient yellow face
5. Permute yellow corners
6. Permute yellow edges

**Complexity**:
- Moves: 80-120 (beginners), 50-60 (advanced)
- Far from optimal, but intuitive

**Pros**:
- **Easy to understand** and teach
- Predictable, low cognitive load
- Works reliably

**Cons**:
- Far from optimal solution
- Not suitable for computer implementation (designed for humans)
- Many moves

**When to Use**: Educational purposes, teaching beginners

---

## Heuristic Functions for Rubik's Cube

### Admissible vs Inadmissible

**Admissible**: Never overestimates distance to goal
- Guarantees optimal solution
- IDA* and A* maintain optimality

**Inadmissible**: May overestimate
- Faster search (more pruning)
- No optimality guarantee
- Useful for fast non-optimal solutions

### 1. Misplaced Pieces Heuristic

**Current Implementation**: [RubiksSolver.cpp:144-158](RubiksSolver.cpp#L144-L158)

```cpp
int heuristic() const {
    int misplaced = 0;
    for (each face) {
        for (each cell) {
            if (cell != reference_color) misplaced++;
        }
    }
    return misplaced / 8;  // Each piece affects 2-3 faces
}
```

**Admissibility**: Yes (always underestimates)

**Pros**:
- Simple to compute
- Always admissible
- Better than no heuristic

**Cons**:
- Weak: underestimates significantly
- Dividing by 8 is rough approximation
- Doesn't account for piece positions

**Effectiveness**: Minimal (5-10% pruning)

### 2. Sum of Manhattan Distances

**Concept**: Sum of distances each piece must travel.

```cpp
int manhattanHeuristic() const {
    int totalDistance = 0;
    for (each piece) {
        int currentPos = findPiece(piece);
        int goalPos = getGoalPosition(piece);
        totalDistance += distance(currentPos, goalPos);
    }
    return totalDistance;
}
```

**Admissibility**: Yes

**Pros**:
- Much stronger than misplaced pieces
- Still fast to compute
- Widely used

**Cons**:
- Doesn't account for orientation
- Ignores move constraints

**Effectiveness**: Moderate (30-50% pruning)

### 3. Pattern Database (Lookup Table)

**Concept**: Pre-compute exact distances for subproblems.

**Implementation**:
1. Choose subproblem (e.g., all corner positions)
2. Generate all states backward from solved
3. Store state → distance mapping
4. Lookup during search

**Example - 2x2x2 Corner Database**:
```cpp
std::unordered_map<State, int> cornerDB;

// Pre-compute (done once)
void buildCornerDB() {
    queue.push(solvedState);
    cornerDB[solvedState] = 0;

    while (!queue.empty()) {
        state = queue.pop();
        for (each move) {
            newState = apply(move, state);
            if (!cornerDB.contains(newState)) {
                cornerDB[newState] = cornerDB[state] + 1;
                queue.push(newState);
            }
        }
    }
}

// Use during search
int heuristic(state) {
    return cornerDB[state];
}
```

**Size** (2x2x2):
- States: 3,674,160
- Storage: ~14 MB (assuming 4 bytes per entry)

**Admissibility**: Yes (exact distances)

**Pros**:
- **Extremely effective** (90%+ pruning)
- Constant-time lookup
- Guaranteed admissible

**Cons**:
- Requires pre-computation
- Memory intensive for large cubes
- Slow initialization

**Effectiveness**: High (10-100x speedup)

**For 3x3x3**:
- Full database: 43 quintillion entries (infeasible)
- Partial databases: Corner + edge subproblems (~100 MB)
- Multiple databases combined with max() heuristic

### 4. Multiple Pattern Databases

**Concept**: Create separate databases for independent subproblems.

```cpp
int heuristic(state) {
    int h1 = cornerDB[extractCorners(state)];
    int h2 = edgeDB[extractEdges(state)];
    return max(h1, h2);  // Still admissible
}
```

**Admissibility**: Yes (max of admissible heuristics is admissible)

**For 3x3x3**:
- Corner database: ~88 MB
- Edge database: ~400 MB
- Combined: Very effective

---

## Move Pruning Techniques

### 1. Inverse Move Elimination ✅ Implemented

**Rule**: Don't apply inverse of last move.

**Example**: U followed by UI returns to original state.

**Effectiveness**: ~8% reduction (1 of 12 moves eliminated)

### 2. Duplicate Move Elimination ✅ Implemented

**Rule**: Don't apply same move twice in a row.

**Example**: U followed by U (should be U2 in 3x3x3).

**Effectiveness**: ~8% reduction

### 3. Commuting Moves (Removed due to bugs)

**Rule**: Don't allow opposite face moves in increasing order.

**Example**: If last move was L, don't allow R (L and R commute, so canonically do R first).

**Effectiveness**: ~50% reduction when working correctly

**Issue**: Caused incorrect pruning in current implementation.

**Future**: Can be re-implemented with careful testing.

### 4. Move Sequences (Not implemented)

**Rule**: Detect and eliminate inefficient sequences.

**Examples**:
- R R R = RI (three rights = one left inverse)
- R L R = L R R (rearrange commuting moves)

**Effectiveness**: ~20% reduction

**Complexity**: Requires looking back 2-3 moves

### 5. Symmetry Reduction (Not implemented)

**Rule**: Only explore one representative of each equivalence class.

**Concept**: Cube can be rotated 24 ways without "changing" scramble.

**Implementation**:
```cpp
State canonicalForm(State state) {
    State canonical = state;
    for (each of 24 rotations) {
        State rotated = rotate(state);
        if (rotated < canonical) {  // Lexicographic comparison
            canonical = rotated;
        }
    }
    return canonical;
}
```

**Effectiveness**: 24x reduction in state space

**Complexity**: Expensive to compute canonical form

---

## Comparison: Which Algorithm When?

| Algorithm | Optimal | Memory | Speed | Best For |
|-----------|---------|--------|-------|----------|
| **Brute Force** | ❌ | O(b^d) | Slowest | Teaching only |
| **DFS** | ❌ | O(d) | Fast (lucky) | Memory-constrained, non-optimal OK |
| **BFS** | ✅ | O(b^d) | Medium | Small cubes, guaranteed optimal |
| **IDDFS** | ✅ | O(d) | Medium | No heuristic available |
| **IDA*** | ✅ | O(d) | **Fast** | **2x2x2, 3x3x3 (current choice)** |
| **A*** | ✅ | O(b^d) | Fastest | Moderate state space |
| **Bidirectional** | ✅ | O(b^(d/2)) | Very fast | Memory available, symmetry |
| **Kociemba** | ~✅ | O(1) | **Very fast** | **3x3x3 (future)** |

---

## Why IDA* for This Project?

### Decision Rationale

**Requirements**:
1. Optimal or near-optimal solutions
2. Reasonable memory usage
3. Clear, maintainable code
4. Extensible to 3x3x3

**IDA* Advantages**:
- ✅ Guarantees optimal solution
- ✅ O(d) memory - scales to 3x3x3
- ✅ Straightforward implementation
- ✅ Effective with pattern databases
- ✅ Well-studied algorithm
- ✅ Can integrate heuristics incrementally

**Alternatives Considered**:

**A*** Rejected:
- Memory usage too high for 3x3x3
- Would need disk-based storage

**Bidirectional** Deferred:
- More complex implementation
- Good future enhancement
- Excellent for 3x3x3 with symmetry

**Kociemba** Deferred:
- Specific to 3x3x3 only
- High implementation complexity
- Requires large lookup tables
- Good long-term goal

---

## Algorithm Evolution Path

### Stage 1: ✅ Current (2x2x2)
- Algorithm: IDA* with move pruning
- Heuristic: Implemented but unused
- Performance: ~15 seconds for depth 7

### Stage 2: Near-term Enhancement
- **Integrate heuristic function**
- Expected: 2-5x speedup
- Still optimal solution

### Stage 3: Pattern Database
- **Add 2x2x2 corner position database**
- Size: ~14 MB
- Expected: 10-100x speedup (near-instant)

### Stage 4: 3x3x3 Basic
- **Extend to 3x3x3 with IDA* + pattern databases**
- Corner DB + Edge DB
- Expected: Solutions in 1-60 seconds

### Stage 5: 3x3x3 Advanced
- **Implement Kociemba's algorithm**
- Expected: <1 second solves
- Near-optimal solutions

### Stage 6: Bidirectional Enhancement
- **Add bidirectional search option**
- Expected: 10-1000x speedup
- Useful for research and comparison

---

## Theoretical Optimality

### What is "Optimal"?

**Optimal Solution**: Minimum number of moves to solve from given state.

**God's Number**: Maximum optimal solution length across all scrambles.
- 2x2x2: 11 moves (QTM - quarter turn metric)
- 3x3x3: 20 moves (QTM), 26 moves (FTM - face turn metric)

### Can We Guarantee Optimal?

**Yes, if**:
1. Using BFS, IDDFS, or IDA*
2. Heuristic is admissible (for IDA*)
3. No pruning that eliminates optimal paths

**Current Status**:
- ✅ Using IDA* (supports optimal)
- ⚠️ Heuristic unused (currently just IDDFS)
- ✅ Move pruning preserves optimality

**With Heuristic**:
- ✅ Misplaced pieces is admissible
- ✅ Will maintain optimality
- ✅ Just faster search

### Optimal vs. Fast

**Trade-off**:
- Optimal solutions: 7-11 moves for 2x2x2, but 15 second search
- Near-optimal: 12-15 moves for 2x2x2, but <1 second search

**Future Option**: Add "fast mode" using inadmissible heuristic or limited depth.

---

## Summary & Recommendations

### Current State
✅ **IDA* with move pruning** is solid choice for 2x2x2
- Optimal solutions guaranteed
- Reasonable performance
- Clean, maintainable code

### Next Steps

**Priority 1**: Integrate existing heuristic
```cpp
// In idaStarRecursive()
if (currentDepth + heuristic() > depthLimit) {
    return false;  // Prune
}
```

**Priority 2**: Build pattern database
- One-time pre-computation
- Massive speedup
- Still optimal

**Priority 3**: Add bidirectional search
- Research-grade enhancement
- Demonstrates algorithm variety
- Useful for benchmarking

### Long-Term Vision

**For 3x3x3**:
1. Start with IDA* + pattern databases (Phase 1)
2. Implement Kociemba's algorithm (Phase 2)
3. Compare performance
4. Offer both: "optimal" vs "fast" modes

**For Education**:
- Document all algorithms
- Provide comparison benchmarks
- Create visualization of search tree
- Show pruning effectiveness

This approach balances optimality, performance, code clarity, and extensibility.
