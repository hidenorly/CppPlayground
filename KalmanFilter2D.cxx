/*
  Copyright (C) 2024 hidenorly

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
#include <array>
#include <cmath>

class KalmanFilter2D {
public:
    KalmanFilter2D(double process_variance, double measurement_variance, 
                   const std::array<double, 2>& initial_position, double initial_error)
        : process_variance_(process_variance),
          measurement_variance_(measurement_variance),
          estimate_{initial_position},
          error_{initial_error, initial_error} {}

    // Update the filter with a new measurement (latitude, longitude)
    std::array<double, 2> update(const std::array<double, 2>& measurement) {
        // Calculate Kalman Gain for latitude and longitude
        double kalman_gain_lat = error_[0] / (error_[0] + measurement_variance_);
        double kalman_gain_lon = error_[1] / (error_[1] + measurement_variance_);

        // Update estimates
        estimate_[0] += kalman_gain_lat * (measurement[0] - estimate_[0]);
        estimate_[1] += kalman_gain_lon * (measurement[1] - estimate_[1]);

        // Update error covariance
        error_[0] = (1 - kalman_gain_lat) * error_[0] + process_variance_;
        error_[1] = (1 - kalman_gain_lon) * error_[1] + process_variance_;

        return estimate_;
    }

private:
    double process_variance_;           // Process noise variance
    double measurement_variance_;       // Measurement noise variance
    std::array<double, 2> estimate_;    // Current estimates for latitude and longitude
    std::array<double, 2> error_;       // Error covariances for latitude and longitude
};

int main() {
    // Parameters
    double process_variance = 1e-4;    // Small process noise
    double measurement_variance = 1e-2; // Measurement noise
    std::array<double, 2> initial_position = {35.6895, 139.6917}; // Initial lat, lon (Tokyo)
    double initial_error = 1.0;

    // Initialize Kalman Filter
    KalmanFilter2D kf(process_variance, measurement_variance, initial_position, initial_error);

    // Example GPS measurements (latitude, longitude)
    std::vector<std::array<double, 2>> measurements = {
        {35.6897, 139.6920},
        {35.6894, 139.6915},
        {35.6896, 139.6919},
        {35.6898, 139.6922},
        {35.6893, 139.6914},
    };

    std::cout << "Measurement\t\tFiltered Estimate\n";
    for (const auto& measurement : measurements) {
        auto filtered = kf.update(measurement);
        std::cout << "(" << measurement[0] << ", " << measurement[1] << ")"
                  << "\t-> (" << filtered[0] << ", " << filtered[1] << ")\n";
    }

    return 0;
}
