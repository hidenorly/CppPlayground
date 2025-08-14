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

// clang++ -std=c++20 Visitor.cxx

#include <iostream>
#include <string>
#include <memory>
#include <variant>
#include <vector>


// Visitor template
template <typename... Ts>
struct Visitor : Ts... {
    using Ts::operator()...;
};
template <typename... Ts> Visitor(Ts...) -> Visitor<Ts...>;


// example
struct Circle {
    double radius;
};

struct Rectangle {
    double width, height;
};

struct Triangle {
    double base, height;
};

using Shape = std::variant<Circle, Rectangle, Triangle>;



int main() {
    std::vector<Shape> shapes;
    shapes.emplace_back(Circle{5.0});
    shapes.emplace_back(Rectangle{4.0, 6.0});
    shapes.emplace_back(Triangle{3.0, 4.0});

    auto area_visitor = Visitor{
        [](const Circle& c) {
            double area = 3.14159 * c.radius * c.radius;
            std::cout << "Circle area: " << area << "\n";
        },
        [](const Rectangle& r) {
            double area = r.width * r.height;
            std::cout << "Rectangle area: " << area << "\n";
        },
        [](const Triangle& t) {
            double area = (t.base * t.height) / 2;
            std::cout << "Triangle area: " << area << "\n";
        }
    };

    for (const auto& shape : shapes) {
        std::visit(area_visitor, shape);
    }
}
