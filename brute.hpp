#include <algorithm>
#include <climits>
#include <limits>
#include <vector>

// Runtime N * M^N; M^N must fit in long long
int brute(int N, const std::vector<int> P, int M) {
    if (N == 0) return 0;

    std::vector<int> load(M, 0);
    int best = INT_MAX;

    long long total = 1;
    for (int i = 0; i < N; i++) total *= M;

    // use N digit base M number, each of the N digits represent which machine the job is assigned to
    for (long long mask = 0; mask < total; mask++) {
        long long x = mask;

        std::fill(load.begin(), load.end(), 0);

        for (int j = 0; j < N; j++) {
            int machine = x % M;
            x /= M;
            load[machine] += P[j];
        }

        int makespan = *std::max_element(load.begin(), load.end());
        best = std::min(best, makespan);
    }

    return best;
}
