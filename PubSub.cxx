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

// clang++ -std=c++20 AbstractFactory.cxx


#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

template <typename T>
class Subscriber {
public:
    virtual ~Subscriber() = default;
    virtual void onEvent(const T& event) = 0;
};

template <typename T>
class Publisher {
protected:
    std::vector<std::weak_ptr<Subscriber<T>>> mSubscribers;

public:
    void subscribe(const std::shared_ptr<Subscriber<T>>& sub) {
        mSubscribers.push_back(sub);
    }

    void unsubscribe(const std::shared_ptr<Subscriber<T>>& sub) {
        mSubscribers.erase(
            std::remove_if(mSubscribers.begin(), mSubscribers.end(),
                           [&](const std::weak_ptr<Subscriber<T>>& wptr) {
                               auto sp = wptr.lock();
                               return !sp || sp == sub;
                           }),
            mSubscribers.end()
        );
    }

    void publish(const T& event) {
        for (auto it = mSubscribers.begin(); it != mSubscribers.end();) {
            if (auto sp = it->lock()) {
                sp->onEvent(event);
                ++it;
            } else {
                it = mSubscribers.erase(it);
            }
        }
    }
};


// examples
class StringSubscriber : public Subscriber<std::string> {
public:
    StringSubscriber() {}
    void onEvent(const std::string& event) override {
        std::cout << event << std::endl;
    }
};

// ---- 実行例 ----
int main() {
    Publisher<std::string> stringPublisher;
    auto sub = std::make_shared<StringSubscriber>();

    stringPublisher.subscribe(sub);

    stringPublisher.publish("Hello, Pub-Sub!");
    stringPublisher.publish("Template-based system");

    return 0;
}
