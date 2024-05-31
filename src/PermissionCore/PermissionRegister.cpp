#include "PermissionCore/PermissionRegister.h"
#include "PermissionCore/PermissionCore.h"
#include <algorithm>
#include <vector>


namespace pmc {

PermissionRegister& PermissionRegister::getInstance() {
    static PermissionRegister instance;
    return instance;
}

// deprecated
bool PermissionRegister::hasPermissionRegisted(const string& pluginName, const int& permissionValue) {
    auto it = mRegisterData.find(pluginName);
    if (it != mRegisterData.end()) {
        auto& permissionVec = it->second;
        return std::any_of(
            permissionVec.begin(),
            permissionVec.end(),
            [&permissionValue](const group::Permission& permission) { return permission.value == permissionValue; }
        );
    }
    return false;
}
bool PermissionRegister::hasPermission(const string& pluginName, const int& permissionValue) {
    auto it = mRegisterData.find(pluginName);
    if (it != mRegisterData.end()) {
        auto& permissionVec = it->second;
        return std::any_of(
            permissionVec.begin(),
            permissionVec.end(),
            [&permissionValue](const group::Permission& permission) { return permission.value == permissionValue; }
        );
    }
    return false;
}

bool PermissionRegister::registerPermission(
    const string& pluginName,
    const int&    permissionValue,
    const string  permissionName
) {
    if (hasPermission(pluginName, permissionValue)) return false;
    mRegisterData[pluginName].push_back({permissionName, permissionValue});
    return true;
}

bool PermissionRegister::unRegisterPermission(const string& pluginName, const int& permissionValue) {
    auto it = mRegisterData.find(pluginName);
    if (it != mRegisterData.end()) {
        auto& permissionVec = it->second;
        auto  permIt =
            std::remove_if(permissionVec.begin(), permissionVec.end(), [&permissionValue](const group::Permission& pe) {
                return pe.value == permissionValue;
            });
        if (permIt != permissionVec.end()) {
            permissionVec.erase(permIt, permissionVec.end());
            return true;
        }
    }
    return false;
}

// deprecated
std::vector<group::Permission> PermissionRegister::getAllPermission(const string& pluginName) {
    auto it = mRegisterData.find(pluginName);
    if (it != mRegisterData.end()) {
        return it->second;
    }
    return {};
}
std::vector<group::Permission> PermissionRegister::getPermissions(const string& pluginName) {
    auto it = mRegisterData.find(pluginName);
    if (it != mRegisterData.end()) {
        return it->second;
    }
    return {};
}

std::optional<group::Permission>
PermissionRegister::getPermission(const string& pluginName, const int& permissionValue) {
    auto it = mRegisterData.find(pluginName);
    if (it != mRegisterData.end()) {
        auto& permissionVec = it->second;
        auto  permIt =
            std::find_if(permissionVec.begin(), permissionVec.end(), [&permissionValue](const group::Permission& pe) {
                return pe.value == permissionValue;
            });
        if (permIt != permissionVec.end()) {
            return *permIt;
        }
    }
    return std::nullopt;
}

string PermissionRegister::getPermissionName(const string& pluginName, const int& permissionValue) {
    auto it = mRegisterData.find(pluginName);
    if (it != mRegisterData.end()) {
        auto& permissionVec = it->second;
        auto  permIt =
            std::find_if(permissionVec.begin(), permissionVec.end(), [&permissionValue](const group::Permission& pe) {
                return pe.value == permissionValue;
            });
        if (permIt != permissionVec.end()) {
            return permIt->name;
        }
    }
    return "";
}

int PermissionRegister::getPermissionValue(const string& pluginName, const string& permissionName) {
    auto it = mRegisterData.find(pluginName);
    if (it != mRegisterData.end()) {
        auto& permissionVec = it->second;
        auto  permIt =
            std::find_if(permissionVec.begin(), permissionVec.end(), [&permissionName](const group::Permission& pe) {
                return pe.name == permissionName;
            });
        if (permIt != permissionVec.end()) {
            return permIt->value;
        }
    }
    return -1;
}

std::vector<string> PermissionRegister::getAllKeys() { // deprecated
    std::vector<string> keys(mRegisterData.size());
    std::transform(mRegisterData.begin(), mRegisterData.end(), keys.begin(), [](const auto& pair) {
        return pair.first;
    });
    return keys;
}
std::vector<string> PermissionRegister::getAllPluginNames() {
    std::vector<string> keys(mRegisterData.size());
    std::transform(mRegisterData.begin(), mRegisterData.end(), keys.begin(), [](const auto& pair) {
        return pair.first;
    });
    return keys;
}

bool PermissionRegister::hasPlugin(const string& pluginName) {
    bool isExist = false;
    for (auto& [key, value] : mRegisterData) {
        if (key == pluginName) {
            isExist = true;
            break;
        }
    }
    return isExist;
}


} // namespace pmc