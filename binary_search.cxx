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

int find_as_binary_search(const std::vector<int>& data, int target){
  int left=0, right = data.size()-1;

  while(left<=right){
    int mid = left + (right-left)/2;
    if( data[mid] == target ){
      return mid;
    } else {
      if( data[mid]<target ){
        left = mid + 1;
      } else {
        right = mid - 1;
      }
    }
  }
  return -1;
}

int main(){
  const std::vector<int> data = {1, 3, 5, 7, 9};
  std::cout << find_as_binary_search(data, 5) << std::endl;;

  const std::vector<int> data2 = {1, 3, 5, 7, 9};
  std::cout << find_as_binary_search(data2, 6) << std::endl;;
}