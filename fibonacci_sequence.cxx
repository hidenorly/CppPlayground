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
#include <ranges>

std::vector<int> fibonacci_sequence(int n)
{
  std::vector<int> result = {0, 1};
  if( n == 0 ) return {};
  if( n == 1 ) return {0};

  for(int i=2; i<n; ++i){
    result.push_back( result[i-1] + result[i-2] );
  }

  return result;
}

int main(int argc, char **argv) {
  int max_num = 30;
  if( argc == 2 ){
    max_num = std::stoi(argv[1]);
  }

  auto fibs = fibonacci_sequence(max_num);
  for (auto f : fibs) {
      std::cout << f << " ";
  }
  std::cout << std::endl;
  return 0;
}