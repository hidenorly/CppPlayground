/*
  Copyright (C) 2025 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <boost/math/distributions/students_t.hpp>

// Calculate the mean of a dataset
double mean(const std::vector<double>& data) {
    if (data.empty()) throw std::invalid_argument("Data cannot be empty.");
    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

// Calculate the variance of a dataset (unbiased)
double variance(const std::vector<double>& data, double mean_value) {
    if (data.size() < 2) throw std::invalid_argument("Data must have at least two elements.");
    double sum_squared_diff = std::accumulate(data.begin(), data.end(), 0.0, [mean_value](double acc, double x) {
        return acc + (x - mean_value) * (x - mean_value);
    });
    return sum_squared_diff / (data.size() - 1);
}

// Calculate the p-value based on the t-statistic and degrees of freedom
double calculate_p_value(double t_statistic, int df) {
    boost::math::students_t dist(df); // Create Student's t-distribution with given degrees of freedom
    double p_value = 2 * (1 - boost::math::cdf(dist, std::abs(t_statistic))); // Two-tailed test
    return p_value;
}

// Perform a two-sample t-test
double t_test(const std::vector<double>& group1, const std::vector<double>& group2) {
    if (group1.size() < 2 || group2.size() < 2) 
        throw std::invalid_argument("Each group must have at least two elements.");
    
    // Calculate means and variances
    double mean1 = mean(group1);
    double mean2 = mean(group2);
    double var1 = variance(group1, mean1);
    double var2 = variance(group2, mean2);

    // Calculate pooled variance
    double pooled_variance = ((var1 / group1.size()) + (var2 / group2.size()));

    // Return the t-statistic
    return (mean1 - mean2) / std::sqrt(pooled_variance);
}

// Calculate degrees of freedom using Welch-Satterthwaite equation
int degrees_of_freedom(const std::vector<double>& group1, const std::vector<double>& group2) {
    double var1 = variance(group1, mean(group1));
    double var2 = variance(group2, mean(group2));

    double numerator = std::pow((var1 / group1.size()) + (var2 / group2.size()), 2);
    double denominator = (std::pow(var1 / group1.size(), 2) / (group1.size() - 1)) +
                         (std::pow(var2 / group2.size(), 2) / (group2.size() - 1));
    
    // Return the calculated degrees of freedom
    return static_cast<int>(numerator / denominator);
}

int main() {
    std::vector<double> groupA = {85, 90, 78, 92, 88};
    std::vector<double> groupB = {76, 81, 79, 80, 77};

    try {
        double t_statistic = t_test(groupA, groupB);
        int df = degrees_of_freedom(groupA, groupB);

        // Calculate the p-value
        double p_value = calculate_p_value(t_statistic, df);

        // Display results
        std::cout << "t-statistic: " << t_statistic << std::endl;
        std::cout << "Degrees of freedom: " << df << std::endl;
        std::cout << "p-value: " << p_value << std::endl;

        // Define the significance level
        double alpha = 0.05; // Commonly used significance level (5%)

        // Make a decision
        if (p_value < alpha) {
            std::cout << "Reject the null hypothesis (p < " << alpha << ")." << std::endl;
        } else {
            std::cout << "Fail to reject the null hypothesis (p >= " << alpha << ")." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
