// Written by Teng Fong

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <numeric>
#include <queue>
#include <unordered_map>
#include <vector>

using namespace std;

int LPT(int N, vector<int> P, int M) {
    priority_queue<int, vector<int>, greater<int>> loadsOfMachines;
    for (int i = 0; i < M; i++) loadsOfMachines.push(0);

    sort(P.begin(), P.end());
    reverse(P.begin(), P.end());

    for (int i = 0; i < N; i++) {
        int leastLoad = loadsOfMachines.top();
        loadsOfMachines.pop();

        loadsOfMachines.push(leastLoad + P[i]);
    }

    int makespan = -1;
    while (!loadsOfMachines.empty()) {
        makespan = max(makespan, loadsOfMachines.top());
        loadsOfMachines.pop();
    }

    return makespan;
}

// -----

bool FFD(int N, vector<int>& P, int M, int currentMakespanGuess) {
    vector<int> loadsOfMachines(M);

    for (int i = 0; i < N; i++) {
        int processingTimeOfCurrentJob = P[i];

        bool foundMachine = false;
        for (int& loadOfCurrentMachine : loadsOfMachines) {
            if (loadOfCurrentMachine + processingTimeOfCurrentJob <= currentMakespanGuess) {
                loadOfCurrentMachine += processingTimeOfCurrentJob;
                foundMachine = true;
                break;
            }
        }

        if (!foundMachine) return false;
    }

    return true;
}

int multifit(int N, vector<int> P, int M) {
    sort(P.begin(), P.end());
    reverse(P.begin(), P.end());

    int totProcessingTime = accumulate(P.begin(), P.end(), 0);
    int mxProcessingTime = *max_element(P.begin(), P.end());

    int lb = max((totProcessingTime + M - 1) / M, mxProcessingTime);
    int ub = totProcessingTime;
    int makespan = totProcessingTime;

    while (lb <= ub) {
        int currentMakespanGuess = lb + (ub - lb) / 2;

        if (FFD(N, P, M, currentMakespanGuess)) {
            makespan = currentMakespanGuess;
            ub = currentMakespanGuess - 1;
        } else {
            lb = currentMakespanGuess + 1;
        }
    }

    return makespan;
}