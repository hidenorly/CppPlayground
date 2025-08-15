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

// clang++ -std=c++20 VisitorOOP.cxx

#include <iostream>
#include <memory>
#include <vector>


// example
struct Circle;
struct Rectangle;
struct Triangle;

// Visitor interface
struct Visitor {
    virtual void visit(const Circle&) = 0;
    virtual void visit(const Rectangle&) = 0;
    virtual void visit(const Triangle&) = 0;
    virtual ~Visitor() = default;
};

// Visitable interface
struct IVisitable {
    virtual void accept(Visitor&) const = 0;
    virtual ~IVisitable() = default;
};


struct Circle : IVisitable {
    double radius;
    Circle(double r) : radius(r) {}
    void accept(Visitor& v) const override { v.visit(*this); }
};

struct Rectangle : IVisitable {
    double width, height;
    Rectangle(double w, double h) : width(w), height(h) {}
    void accept(Visitor& v) const override { v.visit(*this); }
};

struct Triangle : IVisitable {
    double base, height;
    Triangle(double b, double h) : base(b), height(h) {}
    void accept(Visitor& v) const override { v.visit(*this); }
};



int main() {
    std::vector<std::unique_ptr<IVisitable>> shapes;
    shapes.emplace_back(std::make_unique<Circle>(5.0));
    shapes.emplace_back(std::make_unique<Rectangle>(4.0, 6.0));
    shapes.emplace_back(std::make_unique<Triangle>(3.0, 4.0));

    struct MyVisitor : Visitor {
        void visit(const Circle& c) override {
            std::cout << "Circle area: " << (3.14159 * c.radius * c.radius) << "\n";
        }
        void visit(const Rectangle& r) override {
            std::cout << "Rectangle area: " << (r.width * r.height) << "\n";
        }
        void visit(const Triangle& t) override {
            std::cout << "Triangle area: " << ((t.base * t.height) / 2) << "\n";
        }
    };

    MyVisitor visitor;
    for (auto& shape : shapes) {
        shape->accept(visitor);
    }
}
