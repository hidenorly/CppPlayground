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

// clang++ -std=c++20 Template.cxx

#include <iostream>
#include <string>
#include <concepts>


// define interface
template <typename T>
concept HasSteps = requires(T t) {
    { t.step1() } -> std::same_as<void>;
    { t.step2() } -> std::same_as<void>;
};

// CRTP base class
template <typename Derived>
class TemplateMethodBase {
public:
    void run() {
        commonSetup();
        static_cast<Derived*>(this)->step1(); // this->step1(); doesn't work since the template doesn't have.
        static_cast<Derived*>(this)->step2();
        commonCleanup();
    }

protected:
    void commonSetup() {
        std::cout << "[Common] Setup\n";
    }
    void commonCleanup() {
        std::cout << "[Common] Cleanup\n";
    }
};

// Concrete class A
class ConcreteA : public TemplateMethodBase<ConcreteA> {
public:
    void step1() {
        std::cout << "ConcreteA: step1 implementation\n";
    }
    void step2() {
        std::cout << "ConcreteA: step2 implementation\n";
    }
};

// Concrete class B
class ConcreteB : public TemplateMethodBase<ConcreteB> {
public:
    void step1() {
        std::cout << "ConcreteB: custom step1\n";
    }
    void step2() {
        std::cout << "ConcreteB: custom step2\n";
    }
};

// Check the concept
static_assert(HasSteps<ConcreteA>, "ConcreteA must implement step1() and step2()");
static_assert(HasSteps<ConcreteB>, "ConcreteB must implement step1() and step2()");

int main() {
    ConcreteA a;
    std::cout << "--- Run A ---" << std::endl;
    a.run();

    ConcreteB b;
    std::cout << "--- Run B ---" << std::endl;
    b.run();
}
