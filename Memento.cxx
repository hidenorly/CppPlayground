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

// clang++ -std=c++20 Memento.cxx

#include <iostream>
#include <string>
#include <vector>
#include <memory>


template <typename State>
class Memento {
protected:
    State mState;

public:
    explicit Memento(State state) : mState(std::move(state)) {}
    State getState() const { return mState; }
};

template <typename State>
class Originator {
protected:
    State mState;

public:
    void setState(State state) {
        mState = std::move(state);
        std::cout << "State set to: " << mState << "\n";
    }

    State getState() const { return mState; }

    std::shared_ptr<Memento<State>> saveToMemento() const {
        return std::make_shared<Memento<State>>(mState);
    }

    void restoreFromMemento(const std::shared_ptr<Memento<State>>& memento) {
        mState = memento->getState();
        std::cout << "State restored to: " << mState << "\n";
    }
};

template <typename State>
class Caretaker {
protected:
    std::vector<std::shared_ptr<Memento<State>>> mHistory;

public:
    void addMemento(std::shared_ptr<Memento<State>> m) {
        mHistory.push_back(std::move(m));
    }

    std::shared_ptr<Memento<State>> getMemento(size_t index) const {
        if (index < mHistory.size()) {
            return mHistory[index];
        }
        return nullptr;
    }
};

int main() {
    Originator<std::string> originator;
    Caretaker<std::string> caretaker;

    originator.setState("State #1");
    caretaker.addMemento(originator.saveToMemento());

    originator.setState("State #2");
    caretaker.addMemento(originator.saveToMemento());

    originator.setState("State #3");
    caretaker.addMemento(originator.saveToMemento());

    std::cout << "\n--- Undo history ---\n";
    originator.restoreFromMemento(caretaker.getMemento(0));
    originator.restoreFromMemento(caretaker.getMemento(1));

    return 0;
}

