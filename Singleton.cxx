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

// clang++ -std=c++20 Singleton.cxx

#include <iostream>
#include <memory>
#include <mutex>

template <typename T>
class Singleton {
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    static T& getInstance() {
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;
};


// example
class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>;
private:
    Logger() = default;

public:
    void log(const std::string& message) {
        std::cout << message << std::endl;
    }
};


int main() {
    Logger::getInstance().log("Singleton pattern with template");

    return 0;
}
