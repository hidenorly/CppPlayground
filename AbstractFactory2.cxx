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

// clang++ -std=c++20 AbstractFactory2.cxx

#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <string_view>
#include <type_traits>
#include <utility>
#include <iostream>

template <class Base, class T, class... CtorArgs>
concept Creatable =
    std::derived_from<T, Base> && std::constructible_from<T, CtorArgs...>;


template <class Base, class Key = std::string_view, class... CtorArgs>
class FactoryRegistry {
public:
    using Ptr = std::unique_ptr<Base>;
    using CreateFn = Ptr (*)(CtorArgs&&...);

    // --- API ---
    template <class T>
        requires Creatable<Base, T, CtorArgs...>
    bool registerType(const Key& key) {
        std::unique_lock lk(mMutex);
        auto [it, inserted] = mCreators.emplace(key, &FactoryRegistry::template make<T>);
        return inserted;
    }

    bool unregisterType(const Key& key) {
        std::unique_lock lk(mMutex);
        return mCreators.erase(key) > 0;
    }

    bool contains(const Key& key) const {
        std::shared_lock lk(mMutex);
        return mCreators.find(key) != mCreators.end();
    }

    Ptr tryCreate(const Key& key, CtorArgs... args) const {
        std::shared_lock lk(mMutex);
        if (auto it = mCreators.find(key); it != mCreators.end()) {
            return it->second(std::forward<CtorArgs>(args)...);
        }
        return nullptr;
    }

protected:
    template <class T>
        requires Creatable<Base, T, CtorArgs...>
    static Ptr make(CtorArgs&&... args) {
        return std::make_unique<T>(std::forward<CtorArgs>(args)...);
    }

    std::unordered_map<Key, CreateFn> mCreators;
    mutable std::shared_mutex mMutex;
};



// example
struct Button {
    virtual void render() const = 0;
    virtual ~Button() = default;
};

struct WinButton : Button {
    std::string label;
    explicit WinButton(std::string lbl) : label(std::move(lbl)) {}
    void render() const override { std::cout << "[WinButton] " << label << "\n"; }
};

struct MacButton : Button {
    std::string label;
    explicit MacButton(std::string lbl) : label(std::move(lbl)) {}
    void render() const override { std::cout << "[MacButton] " << label << "\n"; }
};

int main() {
    using ButtonFactory = FactoryRegistry<Button, std::string_view, std::string>;

    ButtonFactory btnReg;
    btnReg.registerType<WinButton>("win.button");
    btnReg.registerType<MacButton>("mac.button");

    auto b1 = btnReg.tryCreate("win.button", "OK");
    auto b2 = btnReg.tryCreate("mac.button", "Cancel");
    b1->render();
    b2->render();
}
