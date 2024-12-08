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

int getMaxWindowSum(const std::vector<int>& nums, int windowSize)
{
    int maxSum = 0;
    if(nums.size()<windowSize){
        throw std::invalid_argument("The size of nums should be greater equal than windowSize");
    }

    int windowSum = 0;
    for(int i=0; i<windowSize; i++){
        windowSum += nums[i];
    }

    for(int i=windowSize; i<nums.size(); i++){
        windowSum += nums[i] - nums[i-windowSize];
        maxSum = std::max(windowSum, maxSum);
    }

    return maxSum;
}



int main() {
    try {
        std::vector<int> nums = {1, 4, 2, 10, 23, 3, 1, 0, 20};
        int k = 4;

        std::cout << "Maximum sum of subarray of length " << k 
                  << " is: " << getMaxWindowSum(nums, k) << '\n';
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
    }
    return 0;
}
