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

// clang++ -std=c++20 Strategy.cxx 

#include <iostream>
#include <memory>
#include <string>
#include <functional>


// templated strategy interface
template <typename T>
struct IStrategy {
    virtual ~IStrategy() = default;
    virtual void execute(const T& data) const = 0;
};


// templated concrete
template <typename T>
struct ConsolePrintStrategy : public IStrategy<T> {
    void execute(const T& data) const override {
        std::cout << data << "\n";
    }
};

template <typename T>
struct UpperCasePrintStrategy : public IStrategy<T> {
    void execute(const T& data) const override {
        std::string upper = data;
        for (auto& ch : upper){
            ch = std::toupper(static_cast<unsigned char>(ch));
        }
        std::cout << upper << "\n";
    }
};

// templated context (runtime set strategy)
template <typename T>
class Context {
    std::unique_ptr<IStrategy<T>> strategy;
public:
    Context(std::unique_ptr<IStrategy<T>> s) : strategy(std::move(s)) {}

    void setStrategy(std::unique_ptr<IStrategy<T>> s) {
        strategy = std::move(s);
    }

    void run(const T& data) const {
        if (strategy) strategy->execute(data);
    }
};

template <typename Strategy, typename T>
class StaticContext {
    Strategy strategy;
public:
    void run(const T& data) const {
        strategy.execute(data);
    }
};

int main() {
    // ---- runtime strategy ----
    Context<std::string> ctx(std::make_unique<ConsolePrintStrategy<std::string>>());
    ctx.run("Hello World");
    ctx.setStrategy(std::make_unique<UpperCasePrintStrategy<std::string>>());
    ctx.run("Hello World");

    std::cout << "----\n";

    // ---- compile time strategy ----
    StaticContext<ConsolePrintStrategy<std::string>, std::string> staticCtx1;
    staticCtx1.run("Compile Time Strategy");

    StaticContext<UpperCasePrintStrategy<std::string>, std::string> staticCtx2;
    staticCtx2.run("Compile Time Strategy");
}
