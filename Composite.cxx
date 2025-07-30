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

// clang++ -std=c++20 Composite.cxx

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <concepts>

template <typename T>
class Component {
public:
    virtual ~Component() = default;
    virtual void operation() const = 0;
    virtual void add(std::shared_ptr<Component<T>>) {
        throw std::logic_error("Cannot add to a leaf");
    }
    virtual void remove(std::shared_ptr<Component<T>>) {
        throw std::logic_error("Cannot remove from a leaf");
    }
};

template <typename T>
class Leaf : public Component<T> {
protected:
    T value;
public:
    explicit Leaf(T v) : value(std::move(v)) {}
    void operation() const override {
        std::cout << "Leaf: " << value << "\n";
    }
};

template <typename T>
class Composite : public Component<T> {
protected:
    std::vector<std::shared_ptr<Component<T>>> children;

public:
    void add(std::shared_ptr<Component<T>> component) override {
        children.push_back(component);
    }
    void remove(std::shared_ptr<Component<T>> component) override {
        children.erase(std::remove(children.begin(), children.end(), component), children.end());
    }
    void operation() const override {
        for (const auto& child : children) {
            child->operation();
        }
    }
};

int main() {
    using Comp = Component<std::string>;

    auto root = std::make_shared<Composite<std::string>>();
    auto branch1 = std::make_shared<Composite<std::string>>();
    auto branch2 = std::make_shared<Composite<std::string>>();

    auto leaf1 = std::make_shared<Leaf<std::string>>("A");
    auto leaf2 = std::make_shared<Leaf<std::string>>("B");
    auto leaf3 = std::make_shared<Leaf<std::string>>("C");

    branch1->add(leaf1);
    branch1->add(leaf2);
    branch2->add(leaf3);

    root->add(branch1);
    root->add(branch2);

    root->operation();
}
