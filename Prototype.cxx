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

// clang++ -std=c++20 Prototype.cxx

#include <iostream>
#include <memory>
#include <string>


#include <iostream>
#include <memory>
#include <string>

// Prototype Interface
template <typename Derived>
class Prototype {
public:
    virtual std::unique_ptr<Derived> clone() const = 0;
    virtual void print() const = 0;
    virtual ~Prototype() = default;
};

// Concrete Prototype A
class ConcreteA : public Prototype<ConcreteA> {
    std::string data;

public:
    ConcreteA(std::string d) : data(std::move(d)) {}
    ConcreteA(const ConcreteA& other) = default;

    std::unique_ptr<ConcreteA> clone() const override {
        return std::make_unique<ConcreteA>(*this);
    }

    void print() const override {
        std::cout << "ConcreteA with data: " << data << "\n";
    }
};

// Concrete Prototype B
class ConcreteB : public Prototype<ConcreteB> {
    int value;

public:
    ConcreteB(int v) : value(v) {}
    ConcreteB(const ConcreteB& other) = default;

    std::unique_ptr<ConcreteB> clone() const override {
        return std::make_unique<ConcreteB>(*this);
    }

    void print() const override {
        std::cout << "ConcreteB with value: " << value << "\n";
    }
};

// Factory or usage
int main() {
    std::unique_ptr<Prototype<ConcreteA>> originalA = std::make_unique<ConcreteA>("Hello");
    auto cloneA = originalA->clone();

    std::unique_ptr<Prototype<ConcreteB>> originalB = std::make_unique<ConcreteB>(42);
    auto cloneB = originalB->clone();

    originalA->print();  // ConcreteA with data: Hello
    cloneA->print();     // ConcreteA with data: Hello

    originalB->print();  // ConcreteB with value: 42
    cloneB->print();     // ConcreteB with value: 42
}
