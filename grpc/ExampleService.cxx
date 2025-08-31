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

// clang++ -std=c++20 ExampleService.cxx

#include <iostream>
#include <memory>
#include <string>
#include <map>

#include "ExampleService.hpp"

class MyService : public ServiceBase, public MyInterface
{
protected:
  std::map<std::string, std::string> mRegistry;

public:
  MyService(){
    mRegistry["ro.serialno"] = "dummy";
  }
  virtual ~MyService() = default;

  virtual std::string getValue(std::string key){
    return mRegistry.contains(key) ? mRegistry[key] : "";
  }

  virtual void setValue(std::string key, std::string value){
    mRegistry[key] = value;
  }

};

int main()
{
  MyService service;
  service.setEnabled(true);
  if( service.getEnabled() ){
    std::cout << "service enabled" << std::endl;
    std::cout << "ro.serialno=" << service.getValue("ro.serialno") << std::endl;
    std::cout << "ro.build.fingerprint=" << service.getValue("ro.build.fingerprint") << std::endl;
  }

  return 0;
}