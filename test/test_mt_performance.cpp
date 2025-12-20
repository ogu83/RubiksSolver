#include "RubiksSolver.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <chrono>
#include <vector>

using namespace std;

enum Color { RED, BLUE, ORANGE, GREEN, WHITE, YELLOW, UNDEFINED };
enum Faces { TOP, FRONT, RIGHT, BOTTOM, BACK, LEFT, NONE };
enum Rotation { U, D, R, L, F, B, UI, DI, RI, LI, FI, BI };

// Simple test to show multi-threading speedup
int main() {
    cout << "=== Multi-Threading Performance Test ===" << endl;
    cout << endl;

    // Detect hardware thread count
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;

    cout << "Hardware threads available: " << numThreads << endl;
    cout << endl;

    // Test 1: Single-threaded work
    {
        cout << "Test 1: Single-threaded calculation..." << endl;
        auto start = chrono::high_resolution_clock::now();

        long long sum = 0;
        for (long long i = 0; i < 100000000LL; i++) {
            sum = sum + i;
        }

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

        cout << "Single-threaded time: " << duration.count() << " ms" << endl;
        cout << "Result: " << sum << endl;
        cout << endl;
    }

    // Test 2: Multi-threaded work
    {
        cout << "Test 2: Multi-threaded calculation..." << endl;
        auto start = chrono::high_resolution_clock::now();

        atomic<long long> sum{ 0 };
        vector<thread> threads;

        long long workPerThread = 100000000LL / numThreads;

        for (unsigned int t = 0; t < numThreads; t++) {
            threads.emplace_back([&sum, workPerThread, t]() {
                long long localSum = 0;
                long long startVal = t * workPerThread;
                long long endVal = startVal + workPerThread;

                for (long long i = startVal; i < endVal; i++) {
                    localSum += i;
                }

                sum.fetch_add(localSum);
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

        cout << "Multi-threaded time: " << duration.count() << " ms" << endl;
        cout << "Result: " << sum.load() << endl;
        cout << endl;
    }

    // Test 3: Demonstrate cube solver partitioning
    {
        cout << "Test 3: Cube solver work partitioning demo..." << endl;

        int depth = 3;
        int numMoves = 12;
        long long totalCombinations = 1;
        for (int i = 0; i < depth; i++) {
            totalCombinations *= numMoves;
        }

        cout << "Depth: " << depth << endl;
        cout << "Total combinations: " << totalCombinations << endl;
        cout << "Work per thread (" << numThreads << " threads): "
             << totalCombinations / numThreads << endl;
        cout << endl;

        // Simulate partitioning
        cout << "Thread work distribution:" << endl;
        long long workPerThread = (totalCombinations + numThreads - 1) / numThreads;

        for (unsigned int t = 0; t < numThreads; t++) {
            long long start = t * workPerThread;
            long long end = min(start + workPerThread, totalCombinations);

            if (start < totalCombinations) {
                cout << "  Thread " << t << ": combinations " << start << " to " << (end-1)
                     << " (" << (end - start) << " combinations)" << endl;
            }
        }
        cout << endl;
    }

    // Test 4: Thread creation overhead
    {
        cout << "Test 4: Thread creation overhead test..." << endl;

        auto start = chrono::high_resolution_clock::now();

        vector<thread> threads;
        for (unsigned int t = 0; t < numThreads; t++) {
            threads.emplace_back([]() {
                // Minimal work
                int x = 42;
                (void)x; // Prevent unused warning
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        cout << "Time to create and join " << numThreads << " threads: "
             << duration.count() << " microseconds" << endl;
        cout << endl;
    }

    cout << "=== Summary ===" << endl;
    cout << "Multi-threading is beneficial when:" << endl;
    cout << "1. Work can be partitioned independently" << endl;
    cout << "2. Each partition has enough work to offset thread overhead" << endl;
    cout << "3. Threads don't compete for shared resources (minimize locks)" << endl;
    cout << endl;
    cout << "For Rubik's Cube:" << endl;
    cout << "- Each thread tests different move sequences" << endl;
    cout << "- Threads are independent (each has own cube copy)" << endl;
    cout << "- Only coordinate on solution found (atomic flag + mutex)" << endl;
    cout << "- Expected speedup: " << (numThreads * 0.8) << "x to " << numThreads << "x" << endl;
    cout << endl;

    return 0;
}
