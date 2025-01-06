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

#include "KalmanFilter.hpp"
#include <vector>
#include <iostream>

typedef double LocationInfo;
using KalmanFilter2D = TKalmanFilter<LocationInfo>;


int main() {
    // Parameters
    double processVariance = 1e-4;    // Small process noise
    double measurementVariance = 1e-2; // Measurement noise
    std::vector<LocationInfo> initialPosition = {35.6895, 139.6917}; // Initial lat, lon (Tokyo)
    double initialError = 1.0;

    // Initialize Kalman Filter
    KalmanFilter2D kf(initialPosition, processVariance, measurementVariance, initialError);

    // Example GPS measurements (latitude, longitude)
    std::vector<std::vector<LocationInfo>> measurements = {
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
