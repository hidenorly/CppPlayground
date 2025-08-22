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

// clang++ -std=c++20 PubSubLambda.cxx

#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstddef>


template <typename T>
class Publisher {
    struct Subscriber {
        size_t id;
        std::function<void(const T&)> callback;
    };

    std::vector<Subscriber> subscribers;
    size_t nextId = 0;

public:
    size_t subscribe(std::function<void(const T&)> cb) {
        subscribers.push_back({nextId, std::move(cb)});
        return nextId++;
    }

    void unsubscribe(size_t id) {
        subscribers.erase(
            std::remove_if(subscribers.begin(), subscribers.end(),
                           [&](const Subscriber& s) { return s.id == id; }),
            subscribers.end());
    }

    void publish(const T& event) {
        for (auto& sub : subscribers) {
            sub.callback(event);
        }
    }
};



int main() {
    Publisher<std::string> stringPublisher;

    auto id = stringPublisher.subscribe(
        [](const std::string& msg) { std::cout << msg << "\n"; }
    );

    stringPublisher.publish("Hello Pub-Sub with lambdas!");

    stringPublisher.unsubscribe(id);
    stringPublisher.publish("This should not be published to the id's handler");


    return 0;
}
