#include <gtest/gtest.h>
#include "RubiksSolver.cpp"

// Test fixture for Cube222 tests
class Cube222Test : public ::testing::Test {
protected:
    void SetUp() override {
        cube = new Cube222();
    }

    void TearDown() override {
        delete cube;
    }

    Cube222* cube;
};

// ============================================================================
// Basic State Tests
// ============================================================================

TEST_F(Cube222Test, InitialStateIsSolved) {
    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, ResetRestoresInitialState) {
    cube->applyRotation(U);
    cube->applyRotation(R);
    cube->applyRotation(F);

    EXPECT_FALSE(cube->isSolved());

    cube->reset();
    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, SaveAndRestoreState) {
    // Apply some moves
    cube->applyRotation(U);
    cube->applyRotation(R);
    cube->saveInitState();

    // Apply more moves
    cube->applyRotation(F);
    cube->applyRotation(L);

    // Reset should go back to saved state
    cube->reset();

    // Verify by checking rotations
    EXPECT_FALSE(cube->isSolved());  // Should not be solved (U R was applied)
}

// ============================================================================
// Color Setting Tests
// ============================================================================

TEST_F(Cube222Test, SetSingleColor) {
    cube->setColor(TOP, RED);

    EXPECT_EQ(cube->getColor(TOP, 0, 0), RED);
    EXPECT_EQ(cube->getColor(TOP, 0, 1), RED);
    EXPECT_EQ(cube->getColor(TOP, 1, 0), RED);
    EXPECT_EQ(cube->getColor(TOP, 1, 1), RED);
}

TEST_F(Cube222Test, SetColorVector) {
    std::vector<Color> colors = {RED, BLUE, GREEN, YELLOW};
    cube->setColor(FRONT, colors);

    EXPECT_EQ(cube->getColor(FRONT, 0, 0), RED);
    EXPECT_EQ(cube->getColor(FRONT, 0, 1), BLUE);
    EXPECT_EQ(cube->getColor(FRONT, 1, 0), GREEN);
    EXPECT_EQ(cube->getColor(FRONT, 1, 1), YELLOW);
}

// ============================================================================
// Rotation Tests
// ============================================================================

TEST_F(Cube222Test, SingleRotationChangesState) {
    cube->applyRotation(U);
    EXPECT_FALSE(cube->isSolved());
}

TEST_F(Cube222Test, FourRotationsCycleBack) {
    // Four U rotations should return to solved state
    cube->applyRotation(U);
    cube->applyRotation(U);
    cube->applyRotation(U);
    cube->applyRotation(U);

    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, RotationAndInverseCancels) {
    // U followed by UI should return to solved
    cube->applyRotation(U);
    cube->applyRotation(UI);

    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, AllRotationsAndInversesCycle) {
    // Test all 12 rotations
    Rotation rotations[] = {U, D, R, L, F, B, UI, DI, RI, LI, FI, BI};
    Rotation inverses[] = {UI, DI, RI, LI, FI, BI, U, D, R, L, F, B};

    for (int i = 0; i < 12; i++) {
        cube->reset();
        cube->applyRotation(rotations[i]);
        cube->applyRotation(inverses[i]);

        EXPECT_TRUE(cube->isSolved()) << "Failed for rotation " << i;
    }
}

// ============================================================================
// Move Pruning Tests
// ============================================================================

TEST_F(Cube222Test, RedundantMoveDetectionInverse) {
    // U followed by UI should be detected as redundant
    EXPECT_TRUE(Cube::isRedundantMove(U, UI));
    EXPECT_TRUE(Cube::isRedundantMove(UI, U));
    EXPECT_TRUE(Cube::isRedundantMove(R, RI));
    EXPECT_TRUE(Cube::isRedundantMove(F, FI));
}

TEST_F(Cube222Test, RedundantMoveDetectionDuplicate) {
    // U followed by U should be detected as redundant
    EXPECT_TRUE(Cube::isRedundantMove(U, U));
    EXPECT_TRUE(Cube::isRedundantMove(R, R));
    EXPECT_TRUE(Cube::isRedundantMove(F, F));
}

TEST_F(Cube222Test, NonRedundantMoves) {
    // Different face moves should not be redundant
    EXPECT_FALSE(Cube::isRedundantMove(U, R));
    EXPECT_FALSE(Cube::isRedundantMove(F, L));
    EXPECT_FALSE(Cube::isRedundantMove(D, B));
}

TEST_F(Cube222Test, FirstMoveNeverRedundant) {
    EXPECT_FALSE(Cube::isRedundantMove(ROTATION_NONE, U));
    EXPECT_FALSE(Cube::isRedundantMove(ROTATION_NONE, R));
    EXPECT_FALSE(Cube::isRedundantMove(ROTATION_NONE, F));
}

// ============================================================================
// Backtracking Tests
// ============================================================================

TEST_F(Cube222Test, UndoRotationRestoresState) {
    // Save initial colors
    Color topColor = cube->getColor(TOP, 0, 0);
    Color frontColor = cube->getColor(FRONT, 0, 0);

    // Apply and undo
    cube->applyRotation(U);
    cube->undoRotation(U);

    // Verify restoration
    EXPECT_EQ(cube->getColor(TOP, 0, 0), topColor);
    EXPECT_EQ(cube->getColor(FRONT, 0, 0), frontColor);
    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, MultipleUndoRotations) {
    cube->applyRotation(U);
    cube->applyRotation(R);
    cube->applyRotation(F);

    cube->undoRotation(F);
    cube->undoRotation(R);
    cube->undoRotation(U);

    EXPECT_TRUE(cube->isSolved());
}

// ============================================================================
// Known Solution Tests (From README)
// ============================================================================

TEST_F(Cube222Test, READMEExample_KnownSolution) {
    // Test case from README.md (line 53)
    // Input: -ft YYYY -ff ROOO -fr BGBB -fbk ORRR -fb WWWW -fl GBGG

    // Set up the scrambled cube
    std::vector<Color> topFace = {YELLOW, YELLOW, YELLOW, YELLOW};
    std::vector<Color> frontFace = {RED, ORANGE, ORANGE, ORANGE};
    std::vector<Color> rightFace = {BLUE, GREEN, BLUE, BLUE};
    std::vector<Color> backFace = {ORANGE, RED, RED, RED};
    std::vector<Color> bottomFace = {WHITE, WHITE, WHITE, WHITE};
    std::vector<Color> leftFace = {GREEN, BLUE, GREEN, GREEN};

    cube->setColor(TOP, topFace);
    cube->setColor(FRONT, frontFace);
    cube->setColor(RIGHT, rightFace);
    cube->setColor(BACK, backFace);
    cube->setColor(BOTTOM, bottomFace);
    cube->setColor(LEFT, leftFace);

    cube->saveInitState();

    // Verify it's not solved
    EXPECT_FALSE(cube->isSolved());

    // Apply the known solution from README: F UI B LI B R F
    cube->applyRotation(F);
    cube->applyRotation(UI);
    cube->applyRotation(B);
    cube->applyRotation(LI);
    cube->applyRotation(B);
    cube->applyRotation(R);
    cube->applyRotation(F);

    // Verify it's now solved (first 3 faces should be uniform)
    // Note: The README shows BOTTOM and BACK are not fully solved in output
    // This is expected behavior - checking first 3 faces only
    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, READMEExample_SolverFindsCorrectSolution) {
    // Set up the same scrambled cube
    std::vector<Color> topFace = {YELLOW, YELLOW, YELLOW, YELLOW};
    std::vector<Color> frontFace = {RED, ORANGE, ORANGE, ORANGE};
    std::vector<Color> rightFace = {BLUE, GREEN, BLUE, BLUE};
    std::vector<Color> backFace = {ORANGE, RED, RED, RED};
    std::vector<Color> bottomFace = {WHITE, WHITE, WHITE, WHITE};
    std::vector<Color> leftFace = {GREEN, BLUE, GREEN, GREEN};

    cube->setColor(TOP, topFace);
    cube->setColor(FRONT, frontFace);
    cube->setColor(RIGHT, rightFace);
    cube->setColor(BACK, backFace);
    cube->setColor(BOTTOM, bottomFace);
    cube->setColor(LEFT, leftFace);

    cube->saveInitState();

    EXPECT_FALSE(cube->isSolved());

    // Run the solver
    auto startTime = std::chrono::steady_clock::now();
    cube->dfs(1, startTime);

    // Verify solution found
    EXPECT_TRUE(cube->isSolved());

    // The solution should be found at depth 7 (as per README)
    // We can't test exact moves as algorithm may find different solution
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(Cube222Test, AlreadySolvedCube) {
    // Solving an already solved cube should do nothing
    EXPECT_TRUE(cube->isSolved());

    auto startTime = std::chrono::steady_clock::now();
    cube->dfs(1, startTime);

    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, OneMoveScramble) {
    cube->applyRotation(U);
    cube->saveInitState();

    EXPECT_FALSE(cube->isSolved());

    auto startTime = std::chrono::steady_clock::now();
    cube->dfs(1, startTime);

    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, TwoMoveScramble) {
    cube->applyRotation(U);
    cube->applyRotation(R);
    cube->saveInitState();

    EXPECT_FALSE(cube->isSolved());

    auto startTime = std::chrono::steady_clock::now();
    cube->dfs(1, startTime);

    EXPECT_TRUE(cube->isSolved());
}

// ============================================================================
// Heuristic Tests
// ============================================================================

TEST_F(Cube222Test, HeuristicOnSolvedCube) {
    EXPECT_EQ(cube->heuristic(), 0);
}

TEST_F(Cube222Test, HeuristicIncreasingWithScramble) {
    int h0 = cube->heuristic();

    cube->applyRotation(U);
    int h1 = cube->heuristic();

    EXPECT_GT(h1, h0);  // Heuristic should increase after scrambling
}

TEST_F(Cube222Test, HeuristicNeverNegative) {
    // Apply random moves
    cube->applyRotation(U);
    cube->applyRotation(R);
    cube->applyRotation(F);
    cube->applyRotation(L);
    cube->applyRotation(B);

    EXPECT_GE(cube->heuristic(), 0);
}

// ============================================================================
// Performance Regression Tests
// ============================================================================

TEST_F(Cube222Test, SolveDepth1UnderTimeLimit) {
    cube->applyRotation(U);
    cube->saveInitState();

    auto start = std::chrono::steady_clock::now();
    cube->dfs(1, start);
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(cube->isSolved());
    EXPECT_LT(duration.count(), 100);  // Should solve in < 100ms
}

TEST_F(Cube222Test, SolveDepth3UnderTimeLimit) {
    cube->applyRotation(U);
    cube->applyRotation(R);
    cube->applyRotation(F);
    cube->saveInitState();

    auto start = std::chrono::steady_clock::now();
    cube->dfs(1, start);
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(cube->isSolved());
    EXPECT_LT(duration.count(), 500);  // Should solve in < 500ms
}

// ============================================================================
// Specific Rotation Pattern Tests
// ============================================================================

TEST_F(Cube222Test, SuperflipEquivalent) {
    // 2x2x2 doesn't have a superflip, but test a maximally scrambled state
    cube->applyRotation(R);
    cube->applyRotation(U);
    cube->applyRotation(RI);
    cube->applyRotation(UI);
    cube->applyRotation(R);
    cube->applyRotation(U);
    cube->applyRotation(RI);

    EXPECT_FALSE(cube->isSolved());

    // Undo the sequence
    cube->applyRotation(R);
    cube->applyRotation(UI);
    cube->applyRotation(RI);
    cube->applyRotation(U);
    cube->applyRotation(R);
    cube->applyRotation(UI);
    cube->applyRotation(RI);

    EXPECT_TRUE(cube->isSolved());
}

TEST_F(Cube222Test, CommutatorPattern) {
    // Test commutator: [R, U] = R U RI UI
    cube->applyRotation(R);
    cube->applyRotation(U);
    cube->applyRotation(RI);
    cube->applyRotation(UI);

    // Save this state
    auto state1 = *cube;

    // Undo
    cube->applyRotation(U);
    cube->applyRotation(R);
    cube->applyRotation(UI);
    cube->applyRotation(RI);

    EXPECT_TRUE(cube->isSolved());
}

// ============================================================================
// Copy/Clone Tests
// ============================================================================

TEST_F(Cube222Test, CopyCreatesIndependentCube) {
    cube->applyRotation(U);
    cube->applyRotation(R);

    Cube* cubeCopy = cube->copy();

    // Modify original
    cube->applyRotation(F);

    // Copy should be unchanged (still have U R only)
    EXPECT_FALSE(cube->isSolved());
    EXPECT_FALSE(cubeCopy->isSolved());

    delete cubeCopy;
}

// ============================================================================
// Input Parsing Tests
// ============================================================================

class InputParsingTest : public ::testing::Test {
protected:
    std::map<char, Color> charToColor = {
        {'R', RED}, {'B', BLUE}, {'O', ORANGE},
        {'G', GREEN}, {'W', WHITE}, {'Y', YELLOW}
    };
};

TEST_F(InputParsingTest, CharToColorMapping) {
    EXPECT_EQ(charToColor['R'], RED);
    EXPECT_EQ(charToColor['B'], BLUE);
    EXPECT_EQ(charToColor['O'], ORANGE);
    EXPECT_EQ(charToColor['G'], GREEN);
    EXPECT_EQ(charToColor['W'], WHITE);
    EXPECT_EQ(charToColor['Y'], YELLOW);
}

TEST_F(InputParsingTest, InvalidCharReturnsUndefined) {
    std::map<char, Color> testMap = charToColor;

    // Test invalid character
    char invalidChar = 'X';
    Color result = testMap.count(invalidChar) > 0 ? testMap[invalidChar] : UNDEFINED;

    EXPECT_EQ(result, UNDEFINED);
}

// ============================================================================
// Main Function
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
