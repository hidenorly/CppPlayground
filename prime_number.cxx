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

std::vector<int> generate_primes(int n)
{
  std::vector<int> primes;

  std::vector<bool> is_prime(n + 1, true);
  is_prime[0] = is_prime[1] = false;

  for (int i = 2; i * i <= n; ++i) {
      if (is_prime[i]) {
          for (int j = i * i; j <= n; j += i) {
              is_prime[j] = false;
          }
      }
  }

  for (int i = 2; i <= n; ++i) {
      if (is_prime[i]) {
          primes.push_back(i);
      }
  }

  return primes;
}

int main(int argc, char **argv) {
  int max_num = 30;
  if( argc == 2 ){
    max_num = std::stoi(argv[1]);
  }

  auto primes = generate_primes(max_num);
  for (auto p : primes) {
      std::cout << p << " ";
  }
  return 0;
}