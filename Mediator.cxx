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

// clang++ -std=c++20 Mediator.cxx

#include <iostream>
#include <memory>
#include <string>


// --- template of Mediator
template<typename T>
class Mediator;

template<typename Derived>
class Component {
protected:
    std::shared_ptr<Mediator<Derived>> mediator;

public:
    void setMediator(std::shared_ptr<Mediator<Derived>> med) {
        mediator = med;
    }
};

template<typename T>
class Mediator {
public:
    virtual void notify(T* sender, const std::string& event) = 0;
};
// ---




// examples
class ComponentA;
class ComponentB;

class ComponentA : public Component<ComponentA> {
public:
    void doA() {
        std::cout << "ComponentA does A.\n";
        mediator->notify(this, "A");
    }

    void doX() {
        std::cout << "ComponentA reacts to B's event by doing X.\n";
    }
};

class ComponentB : public Component<ComponentB> {
public:
    void doB() {
        std::cout << "ComponentB does B.\n";
        mediator->notify(this, "B");
    }

    void doY() {
        std::cout << "ComponentB reacts to A's event by doing Y.\n";
    }
};

// Concrete Mediator managing ComponentA and ComponentB
class ConcreteMediator :
    public Mediator<ComponentA>,
    public Mediator<ComponentB>,
    public std::enable_shared_from_this<ConcreteMediator>
{
    std::shared_ptr<ComponentA> compA;
    std::shared_ptr<ComponentB> compB;

public:
    void setComponents(std::shared_ptr<ComponentA> a, std::shared_ptr<ComponentB> b) {
        compA = a;
        compB = b;
        compA->setMediator(shared_from_this());
        compB->setMediator(shared_from_this());
    }

    void notify(ComponentA* sender, const std::string& event) override {
        if (event == "A") {
            compB->doY();
        }
    }

    void notify(ComponentB* sender, const std::string& event) override {
        if (event == "B") {
            compA->doX();
        }
    }
};

int main() {
    auto mediator = std::make_shared<ConcreteMediator>();

    auto a = std::make_shared<ComponentA>();
    auto b = std::make_shared<ComponentB>();

    mediator->setComponents(a, b);

    std::cout << "ComponentA initiates action:\n";
    a->doA();

    std::cout << "\nComponentB initiates action:\n";
    b->doB();
}
