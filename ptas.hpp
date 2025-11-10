// Written by Jie Xiang

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <vector>

// check if a certain config is feasible, i.e. can fit in a bucket of size T
bool feasible(const std::vector<int>& config, double epsilon, int T) {
    int s = config.size();
    double width = (1.0 - epsilon) * T / s;  // bin width
    double curr_left = epsilon * T;          // current left of bin
    double sum = 0;

    for (int i = 0; i < s; i++) {
        sum += config[i] * curr_left;
        curr_left += width;
    }

    return sum <= T && sum > 0;
}

// returns all feasible configs using a counter and then checking to see if sum <= T
std::vector<std::vector<int>> feasible_configs(const std::vector<int>& bin_count, double epsilon, int T) {
    int s = bin_count.size();
    std::vector<int> curr(s, 0);
    std::vector<std::vector<int>> result;

    while (true) {
        if (feasible(curr, epsilon, T)) {
            result.push_back(curr);
        }

        // counter
        int i = s - 1;
        while (i >= 0) {
            curr[i]++;
            if (curr[i] <= bin_count[i]) break;
            curr[i] = 0;
            i--;
        }
        if (i < 0) break;
    }

    return result;
}

// some chatgpt hash for vectors
struct vector_hash {
    size_t operator()(const std::vector<int>& v) const {
        size_t h = 0;
        for (int x : v) {
            h ^= std::hash<int>()(x) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
};

std::vector<int> vector_diff(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> result(a.size(), 0);
    for (size_t i = 0; i < a.size(); i++) {
        result[i] = a[i] - b[i];
    }
    return result;
}

// returns min number of bins needed to pack all jobs into bins of size at most T
int DP(std::unordered_map<std::vector<int>, int, vector_hash>& table, const std::vector<int>& bin_count, double epsilon, int T) {
    if (table.find(bin_count) != table.end()) {
        return table.at(bin_count);
    }

    std::vector<std::vector<int>> confs = feasible_configs(bin_count, epsilon, T);
    int min_val = std::numeric_limits<int>::max();

    for (auto& conf : confs) {
        min_val = std::min(min_val, DP(table, vector_diff(bin_count, conf), epsilon, T));
    }
    table.insert({bin_count, 1 + min_val});

    return 1 + min_val;
}

// get the actual bin packing configs from the DP table
std::vector<std::vector<int>> get_DP_alloc(std::unordered_map<std::vector<int>, int, vector_hash>& table, std::vector<int> bin_count, double epsilon, int T) {
    int s = bin_count.size();
    std::vector<std::vector<int>> result{};

    while (true) {
        int sum = 0;
        for (int i : bin_count) {
            sum += i;
        }
        if (sum == 0) {
            break;
        }

        std::vector<std::vector<int>> confs = feasible_configs(bin_count, epsilon, T);
        int min_val = std::numeric_limits<int>::max();
        std::vector<int> min_alloc(s, 0);

        for (auto& conf : confs) {
            int val = table.at(vector_diff(bin_count, conf));
            if (val < min_val) {
                min_val = val;
                min_alloc = conf;
            }
        }

        bin_count = vector_diff(bin_count, min_alloc);
        result.push_back(std::move(min_alloc));
    }

    return result;
}

// returns a (1 + epsilon)-approximation for makespan
double PTAS(std::vector<int> jobs, int m, double epsilon) {
    double avg_time = 0;
    int max_time = 0;
    for (int t : jobs) {
        avg_time += t;
        max_time = std::max(max_time, t);
    }
    avg_time /= m;

    int lower = std::max((int)std::ceil(avg_time), max_time);
    int upper = 2 * lower;
    int T = (upper + lower) / 2;
    std::vector<int> small_jobs{};
    std::vector<int> large_jobs{};

    int s = std::ceil(1 / (epsilon * epsilon));  // number of bins
    std::vector<int> bin_count(s, 0);

    // DP table
    std::unordered_map<std::vector<int>, int, vector_hash> table{};  // cant be bothered to implement an s dimensional array
    table[std::vector<int>(s, 0)] = 0;                               // base case: 0 jobs can be done with 0 machines

    while (lower < upper) {
        T = (upper + lower) / 2;
        small_jobs.clear();
        large_jobs.clear();

        for (int t : jobs) {
            if (t > epsilon * T) {
                large_jobs.push_back(t);
            } else {
                small_jobs.push_back(t);
            }
        }

        std::vector<int> large_rounded{};

        // we need s bins to cover the interval (epsilon*T, T]
        double width = (1.0 - epsilon) * T / s;  // bin width
        std::fill(bin_count.begin(), bin_count.end(), 0);

        // rounding up to nearest bin
        for (int t : large_jobs) {
            double offset = t - epsilon * T;
            int quotient = offset / width;
            double rem = offset - quotient * width;
            double rounded = epsilon * T + quotient * width;

            if (rem > 0) {
                rounded += width;
                bin_count[quotient]++;
            } else {
                bin_count[quotient - 1]++;
            }

            large_rounded.push_back(rounded);
        }

        table.clear();
        table[std::vector<int>(s, 0)] = 0;  // base case: 0 jobs can be done with 0 machines

        int opt = DP(table, bin_count, epsilon, T);
        if (opt <= m) {
            upper = T;
        } else {
            lower = T + 1;
        }
    }

    std::vector<std::vector<int>> job_alloc_rounded = get_DP_alloc(table, bin_count, epsilon, T);

    // fit actual jobs into rounded bins
    std::vector<std::vector<int>> actual_job_alloc(m, std::vector<int>());
    std::vector<int> makespans(m, 0);
    double width = (1.0 - epsilon) * T / s;  // bin width

    for (int i = 0; i < m; i++) {
        if (job_alloc_rounded.size() == i) {
            break;
        }

        // go through all bins
        for (int j = 0; j < s; j++) {
            if (job_alloc_rounded[i][j] == 0) {
                continue;
            }

            // find k jobs that round to bin j
            for (int k = 0; k < job_alloc_rounded[i][j]; k++) {
                for (size_t l = 0; l < large_jobs.size(); l++) {
                    int t = large_jobs[l];
                    double offset = t - epsilon * T;
                    int quotient = offset / width;
                    double rem = offset - quotient * width;
                    if (rem == 0) {
                        quotient--;
                    }

                    if (quotient == j) {
                        actual_job_alloc[i].push_back(t);
                        makespans[i] += t;

                        // delete t from large_jobs
                        large_jobs[l] = large_jobs.back();
                        large_jobs.pop_back();
                        break;
                    }
                }
            }
        }
    }

    // fit remaining small jobs into least-loaded machine
    for (int t : small_jobs) {
        auto min_it = std::min_element(makespans.begin(), makespans.end());
        auto index = std::distance(std::begin(makespans), min_it);
        *min_it += t;
        actual_job_alloc[index].push_back(t);
    }

    for (int i = 0; i < m; i++) {
        std::cout << "machine " << i << ": ";
        for (auto j : actual_job_alloc[i]) {
            std::cout << j << ' ';
        }
        std::cout << std::endl;
    }

    std::cout << "makespans: ";
    for (auto i : makespans) {
        std::cout << i << ' ';
    }
    std::cout << std::endl;

    auto max_it = std::max_element(makespans.begin(), makespans.end());
    return max_it != makespans.end() ? *max_it : 0;
}

// int main(){
//     // optimal = 12, LPT gives 15
//     std::vector<int> jobs{7,7,6,6,5,5,4,4,4};
//     int makespan = PTAS(jobs, 4, 0.3);
//     std::cout << "PTAS: " << makespan << std::endl;
// }
