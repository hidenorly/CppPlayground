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

// clang++ -std=c++20 EventBus.cxx

#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <algorithm>

class EventBus {
protected:
    class IHolder {
    public:
        virtual ~IHolder() = default;
    };

    template <typename T>
    class Holder : public IHolder {
    protected:
        class Subscriber {
        public:
            size_t id;
            std::function<void(const T&)> callback;

            Subscriber(size_t i, std::function<void(const T&)> cb)
                : id(i), callback(std::move(cb)) {}
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
            for (auto& s : subscribers) {
                s.callback(event);
            }
        }
    };

    std::unordered_map<std::type_index, std::unique_ptr<IHolder>> holders;

    template <typename T>
    Holder<T>& getHolder() {
        auto idx = std::type_index(typeid(T));
        if (!holders.count(idx)) {
            holders[idx] = std::make_unique<Holder<T>>();
        }
        return *static_cast<Holder<T>*>(holders[idx].get());
    }

public:
    template <typename T>
    size_t subscribe(std::function<void(const T&)> cb) {
        return getHolder<T>().subscribe(std::move(cb));
    }

    template <typename T>
    void unsubscribe(size_t id) {
        getHolder<T>().unsubscribe(id);
    }

    template <typename T>
    void publish(const T& event) {
        getHolder<T>().publish(event);
    }
};


int main() {
    EventBus bus;

    auto id1 = bus.subscribe<int>(
        [](const int& v) { std::cout << "[Handler1] v:" << v << "\n"; });

    auto id2 = bus.subscribe<int>(
        [](const int& v) { std::cout << "[Handler2] v*v: " << v * v << "\n"; });

    auto id3 = bus.subscribe<std::string>(
        [](const std::string& msg) { std::cout << "[Handler3] msg: " << msg << "\n"; });

    bus.publish<int>(10);
    bus.publish<std::string>("Hello EventBus");

    bus.unsubscribe<int>(id2);
    bus.publish<int>(7);

    return 0;
}
