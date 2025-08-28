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

// clang++ -std=c++20 PubSubLambda2.cxx

#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <cstddef>

template <typename T>
class Publisher;

template <typename T>
class SubscriberHandle
{
protected:
    std::shared_ptr<Publisher<T>> mPublisher;
    size_t mId;
public:
    SubscriberHandle(std::shared_ptr<Publisher<T>> pub = nullptr, size_t id = 0):mPublisher(pub), mId(id){};
    virtual ~SubscriberHandle(){
        if( mPublisher ){
            mPublisher->unsubscribe( mId );
        }
        mPublisher = nullptr;
    }

};

template <typename T>
class Publisher : public std::enable_shared_from_this<Publisher<T>>
{
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

    std::shared_ptr<SubscriberHandle<T>> subscribeAsHandle(std::function<void(const T&)> cb) {
        auto id = this->subscribe(std::move(cb));
        return std::make_shared<SubscriberHandle<T>>(this->shared_from_this(), id);
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
    std::shared_ptr<Publisher<std::string>> stringPublisher = std::make_shared<Publisher<std::string>>();

    auto id = stringPublisher->subscribe(
            [](const std::string& msg) { std::cout << "#1:" << msg << "\n"; }
        );

    {
        auto id2 = stringPublisher->subscribeAsHandle(
            [](const std::string& msg) { std::cout << "#2:" << msg << "\n"; }
        );

        stringPublisher->publish("Hello Pub-Sub with lambdas!");
    }

    stringPublisher->publish("This should be published to #1 only");

    return 0;
}
