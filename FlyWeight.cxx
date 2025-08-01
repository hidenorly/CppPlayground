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

// clang++ -std=c++20 FlyWeight.cxx

#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <tuple>
#include <mutex>

template <typename T>
inline void hash_combine(std::size_t& seed, const T& value) {
    seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
    template <>
    struct hash<std::tuple<std::string, int>> {
        std::size_t operator()(const std::tuple<std::string, int>& key) const {
            std::size_t seed = 0;
            hash_combine(seed, std::get<0>(key));
            hash_combine(seed, std::get<1>(key));
            return seed;
        }
    };
}


class CharacterGlyph {
protected:
    std::string mFont;
    int mSize;

public:
    CharacterGlyph(std::string font, int size)
    : mFont(std::move(font)), mSize(size) {
        std::cout << "Created Glyph: " << mFont << ", size: " << mSize << "\n";
    }

    void render(char c, int x, int y, const std::string& color) const {
        std::cout << "Render '" << c << "' at (" << x << "," << y
            << ") in font '" << mFont << "' size " << mSize
            << " color " << color << "\n";
    }

    auto key() const {
        return std::make_tuple(mFont, mSize);
    }
};


template <typename T>
class FlyweightFactory {
protected:
    using Key = std::tuple<std::string, int>;  // font, size
    std::unordered_map<Key, std::shared_ptr<T>> mPool;
    std::mutex mMutex;

public:
    template <typename... Args>
    std::shared_ptr<T> get(const std::string& font, int size, Args&&... args) {
        std::lock_guard lock(mMutex);
        Key key{font, size};
        auto it = mPool.find(key);
        if (it != mPool.end()){
            return it->second;
        }

        auto obj = std::make_shared<T>(font, size, std::forward<Args>(args)...);
        mPool[key] = obj;

        return obj;
    }
};

// --- Context class (non-shared part) ---
class Character {
protected:
    char mC;
    int mX, mY;
    std::string mColor;
    std::shared_ptr<CharacterGlyph> mGlyph;

public:
    Character(char c, int x, int y, std::string color, std::shared_ptr<CharacterGlyph> glyph)
    : mC(c), mX(x), mY(y), mColor(std::move(color)), mGlyph(std::move(glyph)) {}

    void draw() const {
        mGlyph->render(mC, mX, mY, mColor);
    }
};


int main() {
    FlyweightFactory<CharacterGlyph> glyphFactory;

    auto glyphA = glyphFactory.get("Arial", 12);
    auto glyphB = glyphFactory.get("Arial", 12);
    auto glyphC = glyphFactory.get("Times New Roman", 14);

    Character ch1('A', 10, 10, "red", glyphA);
    Character ch2('B', 20, 10, "blue", glyphB);
    Character ch3('C', 30, 10, "green", glyphC);

    ch1.draw();
    ch2.draw();
    ch3.draw();

    return 0;
}
