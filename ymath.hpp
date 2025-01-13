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

//#define USE_BOOST

#ifdef USE_BOOST
#include <boost/math/distributions/chi_squared.hpp> // For Chi-squared distribution

// Function to calculate p-value from chi-squared statistic and degrees of freedom
double calculate_p_value(double chi_squared_stat, int degrees_of_freedom) {
    boost::math::chi_squared dist(degrees_of_freedom); // Chi-squared distribution
    double p_value = 1 - boost::math::cdf(dist, chi_squared_stat); // Upper tail probability
    return p_value;
}

#else // USE_BOOST

// Gamma function approximation using Lanczos approximation
double gamma_function(double x) {
    const double coefficients[] = {
        0.99999999999980993, 676.5203681218851, -1259.1392167224028,
        771.32342877765313, -176.61502916214059, 12.507343278686905,
        -0.13857109526572012, 9.9843695780195716e-6, 1.5056327351493116e-7
    };

    if (x < 0.5) {
        return M_PI / (std::sin(M_PI * x) * gamma_function(1 - x));
    }

    x -= 1;
    double a = coefficients[0];
    double t = x + 7.5;
    for (size_t i = 1; i < sizeof(coefficients) / sizeof(double); ++i) {
        a += coefficients[i] / (x + i);
    }

    return std::sqrt(2 * M_PI) * std::pow(t, x + 0.5) * std::exp(-t) * a;
}

// Incomplete gamma function for chi-squared CDF approximation
double incomplete_gamma(double s, double x) {
    double sum = 1.0 / s;
    double term = 1.0 / s;
    for (int n = 1; n < 100; ++n) {
        term *= x / (s + n);
        sum += term;
        if (term < 1e-8) break;
    }
    return sum * std::exp(-x + s * std::log(x) - std::log(gamma_function(s)));
}

// Calculate p-value for chi-squared statistic
double calculate_p_value(double chi_squared_stat, int degrees_of_freedom) {
    return 1.0 - incomplete_gamma(degrees_of_freedom / 2.0, chi_squared_stat / 2.0);
}
#endif // USE_BOOST

