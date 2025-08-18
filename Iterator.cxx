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

// clang++ -std=c++20 Iterator.cxx

#include <iostream>
#include <vector>
#include <memory>
#include <string>

template <typename T>
class Iterator {
public:
    virtual bool hasNext() = 0;
    virtual T getNext() = 0;
    virtual ~Iterator() = default;
};



// example
template <typename T>
class VectorIterator : public Iterator<T> {
private:
    const std::vector<T>& collection;
    size_t index = 0;

public:
    explicit VectorIterator(const std::vector<T>& coll) : collection(coll) {}

    bool hasNext() override {
        return index < collection.size();
    }

    T getNext() override {
        if (!hasNext()) {
            throw std::out_of_range("No more elements");
        }
        return collection[index++];
    }
};


template <typename T>
class Collection
{
protected:
    std::vector<T> mCollection;

public:
    Collection() = default;
    virtual ~Collection() = default;

    void add(const T& value){
        mCollection.push_back( value );
    }

    VectorIterator<T> getIterator(void){
        return VectorIterator<T>(mCollection);
    }
};


// example
int main() {
    Collection<std::string> collection;
    collection.add("a");
    collection.add("b");
    collection.add("c");

    auto it = collection.getIterator();
    while (it.hasNext()) {
        std::cout << it.getNext() << "\n";
    }

    return 0;
}
