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

// clang++ -std=c++20 Builder.cxx 

#include <iostream>
#include <string>
#include <utility>

template<typename T>
class TBuilder {
private:
  T instance;

public:
  template<typename Field, typename Value>
  TBuilder& set(Field T::* field, Value&& value) {
    instance.*field = std::forward<Value>(value);
    return *this;
  }

  T build() {
    return std::move(instance);
  }
};


struct User {
  std::string name;
  std::string email;
};

int main() {
  auto user = TBuilder<User>()
    .set(&User::name, "Harold Twitte")
    .set(&User::email, "twitte.harold@gmail.com")
    .build();

  std::cout << "User: " << user.name << ", " << user.email << std::endl;
}
