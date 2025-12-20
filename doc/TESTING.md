# Testing Guide

## Running Tests

### Compile and Run
```bash
g++ -o RubiksSolverTests RubiksSolverTests.cpp -std=c++20 -fcoroutines
./RubiksSolverTests
```

### Windows
```powershell
g++ -o RubiksSolverTests RubiksSolverTests.cpp -std=c++20 -fcoroutines
.\RubiksSolverTests.exe
```

## Test Coverage

### Current Tests (39 total)

1. **Rotation and Inverse Tests** (6 tests)
   - Verifies each rotation followed by inverse returns to original state
   - Tests: U/UI, D/DI, R/RI, L/LI, F/FI, B/BI

2. **Four Rotation Cycle Tests** (12 tests)
   - Verifies 4 consecutive rotations return to original (360° = 4 × 90°)
   - Tests all 12 rotation types

3. **Known Sequence Tests** (4 tests)
   - Sexy move: (R U R' U')⁶ = identity
   - Other cube algorithms

4. **Specific Rotation Tests** (12 tests)
   - Validates D, DI, and U rotations move pieces correctly
   - Checks each face receives correct colors

5. **README Example Test** ⭐ (4 tests)
   - **Primary regression guard**
   - Tests the solution from README.md works correctly
   - Input: `-ft YYYY -ff ROOO -fr BGBB -fbk ORRR -fb WWWW -fl GBGG`
   - Solution: `F UI B LI B R F` (7 moves)
   - Verifies first 3 faces become uniform

6. **Performance Regression Test** (1 test)
   - Ensures 100 rotations complete in < 100ms
   - Guards against performance degradation

## Using Tests During Optimization

### Workflow

1. **Before optimization**: Run tests to establish baseline
   ```bash
   ./RubiksSolverTests
   # Should show: "39 passed, 0 failed"
   ```

2. **Make optimization changes** to RubiksSolver.cpp

3. **After optimization**: Run tests again
   ```bash
   ./RubiksSolverTests
   ```

4. **Verify results**:
   - ✅ All tests pass → Optimization is safe
   - ❌ Any test fails → Optimization broke something, revert and debug

### Example: Optimizing Move Pruning

```bash
# 1. Baseline
./RubiksSolverTests
# Result: 39 passed, 0 failed ✅

# 2. Edit isRedundantMove() in RubiksSolver.cpp
# ... make changes ...

# 3. Test again
./RubiksSolverTests
# Result: 38 passed, 1 failed ❌
# [FAIL] README example: TOP face is uniform after solution

# 4. Conclusion: Pruning is too aggressive, reverting optimal moves
# 5. Fix the pruning logic and test again
```

## Key Test: README Example

This is the **most important test** for catching regressions:

```cpp
void testREADMEExampleSolution() {
    // Sets up exact cube state from README
    // Applies known solution: F UI B LI B R F
    // Verifies first 3 faces become uniform
}
```

**Why it matters**:
- Tests end-to-end solving capability
- Uses real-world example
- If this fails, core functionality is broken
- Guards against breaking changes during optimization

## Test Results Format

```
========================================
   Rubik's Cube Solver Unit Tests
========================================

=== Test: Rotation followed by inverse returns to original ===
[PASS] U then UI = identity
[PASS] D then DI = identity
...

=== Test: README Example - Known Solution ===
Initial scrambled state from README:
TOP: YY/YY  FRONT: RO/OO  RIGHT: BG/BB  ...
After applying solution (F UI B LI B R F):
TOP: WW/WW  FRONT: BB/BB  RIGHT: OO/OO  ...
[PASS] README example: TOP face is uniform after solution
[PASS] README example: FRONT face is uniform after solution
[PASS] README example: RIGHT face is uniform after solution
[PASS] README example: Solution is 7 moves long

========================================
   Test Results: 39 passed, 0 failed
========================================
```

## Adding New Tests

### Template

```cpp
void testMyFeature() {
    std::cout << "\n=== Test: My Feature ===" << std::endl;

    TestCube cube;

    // Setup
    cube.applyRotation(U);

    // Test and assert
    test(condition, "Description of what should be true");
}
```

### Add to main()

```cpp
int main() {
    // ... existing tests ...
    testMyFeature();  // Add here

    std::cout << "\n========================================" << std::endl;
    std::cout << "   Test Results: " << testsPassed << " passed, "
              << testsFailed << " failed" << std::endl;
    return testsFailed > 0 ? 1 : 0;
}
```

## Continuous Integration (Future)

Once set up with GitHub Actions, tests will run automatically on every commit:

```yaml
# .github/workflows/tests.yml
name: Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Compile tests
        run: g++ -o RubiksSolverTests RubiksSolverTests.cpp -std=c++20 -fcoroutines
      - name: Run tests
        run: ./RubiksSolverTests
```

## Summary

- **39 tests** covering rotations, sequences, and the README example
- **README example test** is the primary regression guard
- **Always run tests** before and after optimization
- **All tests must pass** before committing changes
- Tests ensure optimizations don't break existing functionality

**Remember**: If tests pass, your changes are safe! 🧪✅
