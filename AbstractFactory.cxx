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
#include <memory>
#include <string>

class IGuiComponent
{
public:
    virtual void render() const = 0;
};

class Button : public IGuiComponent
{
public:
    Button(){};
    virtual ~Button() = default;
};

class Checkbox : public IGuiComponent
{
public:
    Checkbox(){};
    virtual ~Checkbox() = default;
};


class WinButton : public Button
{
public:
    WinButton(){};
    virtual ~WinButton() = default;
    void render() const override { std::cout << "Windows Button\n"; }
};

class MacButton : public Button
{
public:
    MacButton(){};
    virtual ~MacButton() = default;
    void render() const override { std::cout << "Mac Button\n"; }
};

class WinCheckbox : public Checkbox
{
public:
    WinCheckbox(){};
    virtual ~WinCheckbox() = default;
    void render() const override { std::cout << "Windows Checkbox\n"; }
};


class MacCheckbox : public Checkbox
{
public:
    MacCheckbox(){};
    virtual ~MacCheckbox() = default;
    void render() const override { std::cout << "Mac Checkbox\n"; }
};


class IGUIFactory
{
public:
    virtual std::shared_ptr<Button> createButton() const = 0;
    virtual std::shared_ptr<Checkbox> createCheckbox() const = 0;
};

class WinFactory : public IGUIFactory {
public:
    WinFactory(){};
    virtual ~WinFactory() = default;

    std::shared_ptr<Button> createButton() const override {
        return std::make_shared<WinButton>();
    }
    std::shared_ptr<Checkbox> createCheckbox() const override {
        return std::make_shared<WinCheckbox>();
    }
};

class MacFactory : public IGUIFactory {
public:
    MacFactory(){};
    virtual ~MacFactory() = default;

    std::shared_ptr<Button> createButton() const override {
        return std::make_shared<MacButton>();
    }
    std::shared_ptr<Checkbox> createCheckbox() const override {
        return std::make_shared<MacCheckbox>();
    }
};

template <typename ButtonType, typename CheckboxType>
    requires std::derived_from<ButtonType, Button> && std::derived_from<CheckboxType, Checkbox>
class TemplatedFactory : public IGUIFactory {
public:
    TemplatedFactory(){};
    virtual ~TemplatedFactory() = default;

    std::shared_ptr<Button> createButton() const override {
        return std::make_shared<ButtonType>();
    }
    std::shared_ptr<Checkbox> createCheckbox() const override {
        return std::make_shared<CheckboxType>();
    }
};

int main() {
    std::shared_ptr<IGUIFactory> factory = std::make_shared<WinFactory>();
    auto button = factory->createButton();
    auto checkbox = factory->createCheckbox();
    button->render();
    checkbox->render();

    std::shared_ptr<IGUIFactory> templatedFactory = std::make_shared<TemplatedFactory<MacButton, MacCheckbox>>();
    auto macButton = templatedFactory->createButton();
    auto macCheckbox = templatedFactory->createCheckbox();
    macButton->render();
    macCheckbox->render();

    return 0;
}
