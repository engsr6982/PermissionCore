#pragma once
#include "Macros.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace perm::registers {

using string = std::string;

struct PermExports Authority {
    string permissionName;
    string permissionValue;
};

// extern std::unordered_map<string, std::vector<Authority>> registerPerm;

PermExports std::vector<Authority> getPluginAllPermissions(const string& pluginName);
PermExports std::optional<Authority> getPluginPermission(const string& pluginName, const string& permissionValue);

PermExports bool isPermissionRegistration(const string& pluginName, const string& permissionValue);

PermExports bool
registerPermission(const string& pluginName, const string& permissionName, const string& permissionValue);

PermExports bool unregisterPermission(const string& pluginName, const string& permissionValue);


} // namespace perm::registers