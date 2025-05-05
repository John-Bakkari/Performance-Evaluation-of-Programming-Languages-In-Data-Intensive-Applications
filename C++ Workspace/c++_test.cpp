#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <charconv>
#include <numeric>
using namespace std;
using namespace chrono;

pair<vector<double>, int> process_file(const string& filename) {
    constexpr double min_val = 1.0;
    constexpr double max_val = 99.0;
    constexpr double range_val = max_val - min_val;
    int anomalies = 0;
    vector<double> transformed_data;
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening: " << filename << endl;
        exit(EXIT_FAILURE);
    }
    string line;
    getline(file, line);
    while (getline(file, line)) {
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        if (pos1 == string::npos || pos2 == string::npos) continue;
        string_view value_str(line.data() + pos2 + 1, line.size() - pos2 - 1);
        if (value_str == "NA") continue;
        double original;
        auto result = from_chars(value_str.data(), value_str.data() + value_str.size(), original);
        if (result.ec == errc() && original >= min_val && original <= max_val) {
            double transformed = (original - min_val) / range_val;
            transformed_data.push_back(transformed);
            if (transformed > 0.9) ++anomalies;
        }
    }
    transformed_data.shrink_to_fit();
    return {transformed_data, anomalies};
}

tuple<double, double, double, string> calculations(const vector<double>& data, 
    const int window_size = 100) {
    const size_t n = data.size();
    double sum_x = 0.0, sum_x2 = 0.0;
    const double* d = data.data();
    for (size_t i = 0; i < n; ++i) {
        const double x = d[i];
        sum_x += x;
        sum_x2 += x * x;
    }
    const double mean = sum_x / n;
    const double variance = (sum_x2 - sum_x*sum_x/n) / n;
    const double std_dev = sqrt(variance);
    vector<double> prefix(n+1, 0.0);
    double* p = prefix.data();
    for (size_t i = 0; i < n; ++i) {
        p[i+1] = p[i] + d[i];
    }
    int increasing = 0, decreasing = 0;
    double prev_mean = 0.0;
    bool first_window = true;
    for (size_t i = 0; i <= n - window_size; ++i) {
        const double current_mean = (p[i+window_size] - p[i]) / window_size;
        if (!first_window) {
            if (current_mean > prev_mean) [[likely]] {
                ++increasing;
            } 
            else if (current_mean < prev_mean) [[unlikely]] {
                ++decreasing;
            }
        }
        prev_mean = current_mean;
        first_window = false;
    }
    const string trend = (increasing > decreasing) ? "increasing" :
    (decreasing > increasing) ? "decreasing" : "stable";
    return {mean, variance, std_dev, trend};
}

int main() {
    vector<string> files = {
        "small_sensor_data_2024.csv",
        "medium_sensor_data_2024.csv",
        "large_sensor_data_2024.csv"
    };

    for (const auto& filename : files) {
        // Process file and time execution
        auto process_start = high_resolution_clock::now();
        auto [data, anomalies] = process_file(filename);
        auto process_end = high_resolution_clock::now();

        // Calculate statistics and time execution
        auto calc_start = high_resolution_clock::now();
        auto [mean, variance, std_dev, trend] = calculations(data);
        auto calc_end = high_resolution_clock::now();

        // Calculate timing metrics
        const double process_time = duration_cast<duration<double>>(process_end - process_start).count();
        const double calc_time = duration_cast<duration<double>>(calc_end - calc_start).count();
        const double total_time = process_time + calc_time;

        // Output test results
        cout << "--- Results for " << filename << " ---\n";
        cout.precision(5);
        cout << fixed
             << "Mean: " << mean << "\n"
             << "Variance: " << variance << "\n"
             << "Standard Deviation: " << std_dev << "\n"
             << "Trend: " << trend << "\n"
             << "Anomalies detected: " << anomalies << "\n"
             << "Processing time: " << process_time << "s\n"
             << "Calculation time: " << calc_time << "s\n"
             << "Total time: " << total_time << "s\n\n";
    }
    return 0;
}