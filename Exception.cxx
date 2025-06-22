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

// clang++ -std=c++20 Exception.cxx 

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <source_location>
#include <sstream>
#include <cxxabi.h>

enum class ExceptionSeverity {
    Recoverable,
    Critical
};

class BaseException : public std::exception {
protected:
    std::string mMsg;
    ExceptionSeverity mSeverity;
    std::source_location mLocation;


public:
    BaseException(std::string_view message,
                  ExceptionSeverity severity,
                  std::source_location location = std::source_location::current())
        : mMsg(message),
          mSeverity(severity),
          mLocation(location) {}

    const char* what() const noexcept override {
        return mMsg.c_str();
    }

    ExceptionSeverity severity() const noexcept {
        return mSeverity;
    }

    bool isRecoverable() const noexcept {
        return mSeverity == ExceptionSeverity::Recoverable;
    }

    const std::source_location& location() const noexcept {
        return mLocation;
    }

	std::string demangle(const char* name) const {
		int status = 0;
		char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
		std::string result = (status == 0 && demangled) ? demangled : name;
		free(demangled);
		return result;
	}

    void dump(std::ostream& os = std::cerr) const {
		os << demangle(typeid(*this).name()) << ": "
			<< what() << " ("
			<< (isRecoverable() ? "RECOVERABLE" : "CRITICAL") << ")\n\t"
			<< location().file_name() << ':' << location().line()
			<< std::endl;
    }
};

class InvalidArgumentException final : public BaseException {
public:
    explicit InvalidArgumentException(std::string_view message,
        std::source_location location = std::source_location::current())
        : BaseException(message, ExceptionSeverity::Recoverable, location) {}
};

class IllegalStateException final : public BaseException {
public:
    explicit IllegalStateException(std::string_view message,
        std::source_location location = std::source_location::current())
        : BaseException(message, ExceptionSeverity::Critical, location) {}
};


int main(int argc, char** argv) {
	std::cout << "throw InvalidArgumentException" << std::endl;
    try {
        throw InvalidArgumentException("example invalid arg");
    } catch (const BaseException& ex) {
    	ex.dump();
        if (!ex.isRecoverable()) throw;
    }

	std::cout << "\nthrow IllegalStateException" << std::endl;
    try {
        throw IllegalStateException("example failure");
    } catch (const BaseException& ex) {
    	ex.dump();
        if (!ex.isRecoverable()) throw;
    }

	return 0;
}