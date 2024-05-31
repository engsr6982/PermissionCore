#pragma once
#include "Group.h"
#include "Macros.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace pmc {

using string = std::string;

class PermExports PermissionRegister {
public:
    static PermissionRegister& getInstance();

    // 弃用
    [[deprecated]] bool hasPermissionRegisted(const string& pluginName, const int& permissionValue);

    [[deprecated]] std::vector<group::Permission> getAllPermission(const string& pluginName);

    [[deprecated]] std::vector<string> getAllKeys();


    std::vector<string> getAllPluginNames();

    bool hasPlugin(const string& pluginName);

    bool hasPermission(const string& pluginName, const int& permissionValue);

    bool registerPermission(const string& pluginName, const int& permissionValue, const string permissionName);

    bool unRegisterPermission(const string& pluginName, const int& permissionValue);

    std::optional<group::Permission> getPermission(const string& pluginName, const int& permissionValue);

    std::vector<group::Permission> getPermissions(const string& pluginName);

    string getPermissionName(const string& pluginName, const int& permissionValue);

    int getPermissionValue(const string& pluginName, const string& permissionName);

private:
    PermissionRegister() = default;
    std::unordered_map<string, std::vector<group::Permission>> mRegisterData;
};

} // namespace pmc