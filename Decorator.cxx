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

// clang++ -std=c++20 Decorator.cxx

#include <iostream>
#include <memory>
#include <string>

class IMessage {
public:
	virtual ~IMessage() = default;
	virtual std::string getContent() const = 0;
};

// ConcreteComponent
class TextMessage : public IMessage {
protected:
	std::string mMsg;

public:
	explicit TextMessage(std::string msg) : mMsg(std::move(msg)) {}

	std::string getContent() const override {
		return mMsg;
	}
};

// Decorator Base (using template)
template <typename T>
class MessageDecorator : public IMessage {
protected:
	std::shared_ptr<T> component;

public:
	explicit MessageDecorator(std::shared_ptr<T> comp) : component(std::move(comp)) {}
};

// ConcreteDecorator: adds HTML encoding
class HtmlEncodedMessage : public MessageDecorator<IMessage> {
public:
	using MessageDecorator::MessageDecorator;

	std::string getContent() const override {
		std::string raw = component->getContent();
		// TODO: other conversions
		size_t pos = 0;
		while ((pos = raw.find("&", pos)) != std::string::npos) {
			raw.replace(pos, 1, "&amp;");
			pos += 5;
		}
		return "<html>" + raw + "</html>";
	}
};

// ConcreteDecorator: adds encryption (dummy)
class EncryptedMessage : public MessageDecorator<IMessage> {
public:
	using MessageDecorator::MessageDecorator;

	std::string getContent() const override {
		std::string raw = component->getContent();
		for (char& c : raw) {
			c += 1;
		}
		return raw;
	}
};

int main() {
    std::shared_ptr<IMessage> msg = std::make_shared<TextMessage>("Hello & Welcome");

    // Encrypt -> HTML encode
    msg = std::make_shared<EncryptedMessage>(msg);
    msg = std::make_shared<HtmlEncodedMessage>(msg);

    std::cout << "Decorated Message: " << msg->getContent() << std::endl;
}
