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
#include <stdexcept>
#include <boost/math/distributions/chi_squared.hpp> // For Chi-squared distribution

// Function to calculate chi-squared statistic
double calculate_chi_squared(const std::vector<double>& observed, const std::vector<double>& expected) {
    if (observed.size() != expected.size()) {
        throw std::invalid_argument("Observed and expected vectors must have the same size.");
    }

    double chi_squared = 0.0;
    for (size_t i = 0; i < observed.size(); ++i) {
        if (expected[i] == 0) {
            throw std::invalid_argument("Expected values must be non-zero.");
        }
        double diff = observed[i] - expected[i];
        chi_squared += (diff * diff) / expected[i];
    }
    return chi_squared;
}

// Function to calculate p-value from chi-squared statistic and degrees of freedom
double calculate_p_value(double chi_squared_stat, int degrees_of_freedom) {
    boost::math::chi_squared dist(degrees_of_freedom); // Chi-squared distribution
    double p_value = 1 - boost::math::cdf(dist, chi_squared_stat); // Upper tail probability
    return p_value;
}

int main() {
    // Example data
    std::vector<double> observed = {50, 30, 20};
    std::vector<double> expected = {40, 40, 20};

    try {
        // Calculate chi-squared statistic
        double chi_squared_stat = calculate_chi_squared(observed, expected);
        int degrees_of_freedom = observed.size() - 1; // Degrees of freedom (number of categories - 1)

        // Calculate p-value
        double p_value = calculate_p_value(chi_squared_stat, degrees_of_freedom);

        // Output results
        std::cout << "Chi-squared statistic: " << chi_squared_stat << std::endl;
        std::cout << "Degrees of freedom: " << degrees_of_freedom << std::endl;
        std::cout << "p-value: " << p_value << std::endl;

        // Define the significance level
        double alpha = 0.05;

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
