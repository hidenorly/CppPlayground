/*
  Copyright (C) 2026 hidenorly

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

#ifndef __SAFE_FLAG_HPP__
#define __SAFE_FLAG_HPP__

#include <stdint.h>

enum class SafeFlagStatus : uint8_t
{
    VALID,
    CORRECTED,
    FAULT
};

template <typename T>
class SafeFlag
{
private:
    volatile T mValue1;
    volatile T mValue2;
    volatile T mValue3;

public:
    SafeFlag() noexcept
        : mValue1{}, mValue2{}, mValue3{} {
    }

    explicit SafeFlag(const T& value) noexcept
        : mValue1{value}, mValue2{value}, mValue3{value} {
    }

    void set(const T& value) noexcept{
        mValue1 = value;
        mValue2 = value;
        mValue3 = value;
    }

    SafeFlagStatus get(T& out) const noexcept {
        if (mValue1 == mValue2) {
            out = mValue1;
            return SafeFlagStatus::VALID;
        }

        if (mValue1 == mValue3) {
            out = mValue1;
            return SafeFlagStatus::CORRECTED;
        }

        if (mValue2 == mValue3) {
            out = mValue2;
            return SafeFlagStatus::CORRECTED;
        }

        return SafeFlagStatus::FAULT;
    }
};

#endif // __SAFE_FLAG_HPP__
