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
#include <unordered_map>
#include <map>

std::vector<int> get_k_top_frequent(const std::vector<int>& data, int k){
  std::vector<int> result;

  std::unordered_map<int, int> freqMap;
  for(auto num : data){
    freqMap[num]++;
  }

  std::map<int, std::vector<int>> sortedMap;
  for(auto [num, count] : freqMap){
    sortedMap[count].push_back(num);
  }

  int count=0;
  for(auto it=sortedMap.rbegin(); it!=sortedMap.rend() && count<k; it++){
    for(auto num : it->second){
      result.push_back(num);
      count++;
    }
  }

  return result;
}

int main() {
    std::vector<int> data = {3,3,3, 1, 1, 1, 2, 2, 3};
    int k = 2;

    std::vector<int> result = get_k_top_frequent(data, k);

    for (int num : result) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}