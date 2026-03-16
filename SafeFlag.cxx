/*
  Copyright (C) 2026 hidenorly

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

// clang++ -std=c++20 SafeFlag.cxx

#include "SafeFlag.hpp"
#include <iostream>


enum class FeatureFlag : uint8_t
{
    DISABLED = 0,
    ENABLED  = 1
};

SafeFlag<FeatureFlag> camera_feature(FeatureFlag::DISABLED);

int main() {
    FeatureFlag flag;
    SafeFlagStatus status = camera_feature.get(flag);

    if (status == SafeFlagStatus::FAULT) {
        std::cout << "camera_feature is corrupt." << std::endl;
    } else {
        if (flag == FeatureFlag::ENABLED) {
            std::cout << "camera_feature is enabled!" << std::endl;
        } else {
            std::cout << "camera_feature is disabled!" << std::endl;
        }
    }
    return 0;
}
