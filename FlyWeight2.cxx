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

// clang++ -std=c++20 FlyWeight2.cxx

#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <algorithm>
#include <cctype>
#include <typeindex>

template <typename ContextType>
class Context
{
public:
    ContextType context;
    Context() = default;
    virtual ~Context() = default;
};


template <typename ContextType>
class FlyWeight
{
public:
    FlyWeight() = default;
    virtual ~FlyWeight() = default;
    virtual void operation(std::shared_ptr<Context<ContextType>> context) = 0;
};


template <typename ContextType>
class FlyweightFactory {
protected:
    std::unordered_map<std::type_index, std::shared_ptr<FlyWeight<ContextType>>> mPool;
    std::mutex mMutex;

public:
    template <typename T, typename... Args>
    std::shared_ptr<FlyWeight<ContextType>> get(Args&&... args) {
        std::lock_guard lock(mMutex);
        std::type_index key(typeid(T));
        auto it = mPool.find(key);
        if (it != mPool.end()){
            return it->second;
        }

        auto obj = std::make_shared<T>(std::forward<Args>(args)...);
        mPool[key] = obj;

        return obj;
    }
};


using MyContextType = std::string;
using MyContext = Context<MyContextType>;
using MyFlyWeight = FlyWeight<MyContextType>;
using MyFlyweightFactory = FlyweightFactory<MyContextType>;

class OperatorUpper : public MyFlyWeight
{
public:
    OperatorUpper() = default;
    virtual ~OperatorUpper() = default;
    virtual void operation(std::shared_ptr<MyContext> context){
        std::ranges::transform(context->context, context->context.begin(), [](unsigned char c) {
            return std::toupper(c);
        });
    };
};

class OperatorLower : public MyFlyWeight
{
public:
    OperatorLower() = default;
    virtual ~OperatorLower() = default;
    virtual void operation(std::shared_ptr<MyContext> context){
        std::ranges::transform(context->context, context->context.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
    };
};


int main() {
    std::shared_ptr<MyContext> context = std::make_shared<MyContext>();
    context->context = "hello world";
    MyFlyweightFactory factory;

    auto flyweight = factory.get<OperatorUpper>();
    flyweight->operation(context);
    std::cout << context->context << std::endl;

    auto flyweight2 = factory.get<OperatorLower>();
    flyweight2->operation(context);
    std::cout << context->context << std::endl;

    auto flyweight3 = factory.get<OperatorUpper>(); // it should be same instance with flyweight
    std::cout << (flyweight == flyweight3 ? "same instance" : "different instance") << std::endl;
    flyweight3->operation(context);
    std::cout << context->context << std::endl;

    return 0;
}
