#pragma once
#include "Group.h"
#include "Macros.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace perm {

using string = std::string;

class PermExports PermissionRegister {
public:
    static PermissionRegister& getInstance();

    bool hasPermissionRegisted(const string& pluginName, const int& permissionValue);
    bool registerPermission(const string& pluginName, const int& permissionValue, const string permissionName);
    bool unRegisterPermission(const string& pluginName, const int& permissionValue);

    std::vector<group::Permission>   getAllPermission(const string& pluginName);
    std::optional<group::Permission> getPermission(const string& pluginName, const int& permissionValue);
    string                           getPermissionName(const string& pluginName, const int& permissionValue);
    int                              getPermissionValue(const string& pluginName, const string& permissionName);

    std::vector<string> getAllKeys();

private:
    PermissionRegister() = default;
    std::unordered_map<string, std::vector<group::Permission>> mRegisterData;
};

} // namespace perm