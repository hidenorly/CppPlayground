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

// clang++ -std=c++20 Proxy.cxx

#include <iostream>
#include <string>
#include <memory>
#include <map>

class PackageInfo
{
public:
    PackageInfo() = default;
    virtual ~PackageInfo() = default;

    std::string packageName;
    std::string installedPath;
    std::map<std::string, std::string> meta;
};

class IPackageManager
{
public:
    virtual bool installPackage(std::string packagePath) = 0;
    virtual std::vector<std::shared_ptr<PackageInfo>> getPackageList() = 0;
};


// Internal impl.
class PackageManagerServiceInternal : public IPackageManager
{
protected:
    std::map<std::string, std::shared_ptr<PackageInfo>> mPackages;

public:
    PackageManagerServiceInternal() = default;
    virtual ~PackageManagerServiceInternal() = default;

    std::string getPackageName(std::string packagePath){
        int nPos = packagePath.rfind("/");
        if( nPos!= std::string::npos ){
            return packagePath.substr(nPos+1);
        }

        return packagePath;
    }

    std::string getInstallPath(std::string packagePath){
        std::string path = getPackageName(packagePath);
        return "/data/app/" + path;
    }

    std::shared_ptr<PackageInfo> getPackageInfo(std::string packagePath){
        std::shared_ptr<PackageInfo> info = std::make_shared<PackageInfo>();
        info->packageName = getPackageName(packagePath);
        info->installedPath = getInstallPath(packagePath);
        return info;
    }

    virtual bool installPackage(std::string packagePath){
        std::shared_ptr<PackageInfo> info = getPackageInfo(packagePath);
        mPackages[ info->packageName ] = info;
        return true;
    }
    virtual std::vector<std::shared_ptr<PackageInfo>> getPackageList(){
        std::vector<std::shared_ptr<PackageInfo>> packageInfoList;
        for( auto& [packageName, packageInfo] : mPackages ){
            packageInfoList.push_back(packageInfo);
        }
        return packageInfoList; 
    }
};


// Proxy pattern
class PackageManagerService : public IPackageManager
{
protected:
    std::shared_ptr<IPackageManager> mPackageManager;

    static std::shared_ptr<PackageManagerService> mInstance;


public:
    PackageManagerService(){
        mPackageManager = std::make_shared<PackageManagerServiceInternal>();
    }

    virtual ~PackageManagerService() = default;

    static std::shared_ptr<PackageManagerService> getInstance(){
        if( !mInstance ){
            mInstance = std::make_shared<PackageManagerService>();
        }
        return mInstance;
    }

    bool checkCallerPermission(){
        // TODO: check caller permission
        return true;
    }

    virtual bool installPackage(std::string packagePath){
        if( checkCallerPermission() && mPackageManager ){
            return mPackageManager->installPackage( packagePath );
        }
        return false;

    }
    virtual std::vector<std::shared_ptr<PackageInfo>> getPackageList(){
        if( checkCallerPermission() && mPackageManager ){
            return mPackageManager->getPackageList();
        }
        std::vector<std::shared_ptr<PackageInfo>> nullResult;
        return nullResult;
    }
};

std::shared_ptr<PackageManagerService> PackageManagerService::mInstance;


int main() {
    std::shared_ptr<PackageManagerService> packageManager = PackageManagerService::getInstance();
    packageManager->installPackage("/data/download/browser.apk");
    packageManager->installPackage("/data/download/gallery.apk");

    std::vector<std::shared_ptr<PackageInfo>> packageInfos = packageManager->getPackageList();
    for(auto& packageInfo : packageInfos){
        std::cout << packageInfo->packageName << " : " << packageInfo->installedPath << std::endl;
    }

    return 0;
}
