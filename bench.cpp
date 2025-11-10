#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "brute.hpp"
#include "ptas.hpp"
#include "temp.hpp"

vector<int> generate_jobs(int n, int max_time) {
    std::vector<int> jobs;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, max_time);

    for (int i = 0; i < n; i++) {
        jobs.push_back(dis(gen));
    }

    return jobs;
}

void run_benchmarks(int n, int m, double epsilon, int num_tries) {
    vector<std::chrono::duration<double>> lpt_times, multifit_times, ptas_times, brute_times;
    vector<double> lpt_approx, multifit_approx, ptas_approx;
    double lpt_makespan = 0, multifit_makespan = 0, ptas_makespan = 0, brute_makespan = 0;
    while (num_tries--) {
        std::vector<int> jobs = generate_jobs(n, 10000);
        cout << "Jobs: ";
        for (auto j : jobs) {
            cout << j << ' ';
        }
        cout << endl;

        auto brute_start = std::chrono::high_resolution_clock::now();
        brute_makespan = brute(n, jobs, m);
        auto brute_end = std::chrono::high_resolution_clock::now();
        brute_times.push_back(brute_end - brute_start);

        auto lpt_start = std::chrono::high_resolution_clock::now();
        lpt_makespan = LPT(n, jobs, m);
        auto lpt_end = std::chrono::high_resolution_clock::now();
        lpt_times.push_back(lpt_end - lpt_start);
        lpt_approx.push_back((double)lpt_makespan / brute_makespan);

        auto multifit_start = std::chrono::high_resolution_clock::now();
        multifit_makespan = multifit(n, jobs, m);
        auto multifit_end = std::chrono::high_resolution_clock::now();
        multifit_times.push_back(multifit_end - multifit_start);
        multifit_approx.push_back((double)multifit_makespan / brute_makespan);

        auto ptas_start = std::chrono::high_resolution_clock::now();
        ptas_makespan = PTAS(jobs, m, epsilon);
        auto ptas_end = std::chrono::high_resolution_clock::now();
        ptas_times.push_back(ptas_end - ptas_start);
        ptas_approx.push_back((double)ptas_makespan / brute_makespan);

        cout << "Brute: " << brute_makespan << ", LPT: " << lpt_makespan << ", Multifit: " << multifit_makespan << ", PTAS: " << ptas_makespan << std::endl;
    }

    auto average_time = [](const vector<std::chrono::duration<double>>& times) {
        std::chrono::duration<double> total(0);
        for (const auto& t : times) {
            total += t;
        }
        return total.count() / times.size();
    };

    auto average_approximation = [](const vector<double>& approximations) {
        double t = 0.0;
        for (const auto& a : approximations) {
            t += a;
        }
        return t / approximations.size();
    };

    auto std_dev_approximation = [](const vector<double>& approximations, double mean) {
        double sum = 0.0;
        for (const auto& a : approximations) {
            sum += (a - mean) * (a - mean);
        }
        return std::sqrt(sum / approximations.size());
    };

    std::cout << "Benchmark results for num_jobs=" << n << ", num_machines=" << m << ", epsilon=" << epsilon << ":\n";
    std::cout << "LPT average time: " << average_time(lpt_times) << " avg approximation: " << average_approximation(lpt_approx) << " max approximation: " << *std::max_element(lpt_approx.begin(), lpt_approx.end()) << " std dev: " << std_dev_approximation(lpt_approx, average_approximation(lpt_approx)) << std::endl;
    std::cout << "Multifit average time: " << average_time(multifit_times) << " avg approximation: " << average_approximation(multifit_approx) << " max approximation: " << *std::max_element(multifit_approx.begin(), multifit_approx.end()) << " std dev: " << std_dev_approximation(multifit_approx, average_approximation(multifit_approx)) << std::endl;
    std::cout << "PTAS average time: " << average_time(ptas_times) << " avg approximation: " << average_approximation(ptas_approx) << " max approximation: " << *std::max_element(ptas_approx.begin(), ptas_approx.end()) << " std dev: " << std_dev_approximation(ptas_approx, average_approximation(ptas_approx)) << std::endl;
    std::cout << "Brute-force average time: " << average_time(brute_times) << std::endl
              << std::endl;
}

int main() {
    // std::vector<int> job_counts = {1000, 10000, 100000, 1000000, 10000000};
    std::vector<int> job_counts = {10};
    std::vector<int> machine_counts = {4};
    double epsilon = 0.1;

    for (int n : job_counts) {
        for (int m : machine_counts) {
            run_benchmarks(n, m, epsilon, 100);
        }
    }
    return 0;
}