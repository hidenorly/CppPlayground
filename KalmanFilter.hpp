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

#include <vector>

template<typename DataType> class TKalmanFilter {
protected:
    double mProcessVariance;            // Process noise variance
    double mMeasurementVariance;        // Measurement noise variance
    double mInitialError;               // Initial_error
    std::vector<DataType> mEstimates;   // Current estimates
    std::vector<double> mErrors;        // Error covariances

public:
    TKalmanFilter(double processVariance, double measurementVariance, 
                   const std::vector<DataType>& initialPosition, double initialError)
        : mProcessVariance(processVariance),
          mMeasurementVariance(measurementVariance),
          mEstimates(initialPosition),
          mInitialError(initialError) {}
    virtual ~TKalmanFilter() = default;

    // Update the filter with a new measurement
    std::vector<DataType> update(const std::vector<DataType>& measurement) {
        if( mErrors.empty() ){
            for(int i=0; i<measurement.size(); i++){
                mErrors.push_back(mInitialError);
            }
        }
        int i=0;
        for(auto aMeasurement : measurement){
            double kalman_gain = mErrors[i] / (mErrors[i] + mMeasurementVariance);
            mEstimates[i] += kalman_gain * (aMeasurement - mEstimates[i]);
            // Update error covariance
            mErrors[i] = (1 - kalman_gain) * mErrors[i] + mProcessVariance;
            i++;
        }

        return mEstimates;
    }
};
