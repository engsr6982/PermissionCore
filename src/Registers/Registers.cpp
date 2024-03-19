#include "include_all.h"
#include <algorithm>


namespace perm::registers {

std::unordered_map<string, std::vector<Authority>> registerPerm;

const std::vector<Authority>& getPluginAllPermissions(const string& pluginName) {
    if (registerPerm.find(pluginName) != registerPerm.end()) {
        return registerPerm[pluginName];
    }
    return {};
}

const std::optional<Authority>& getPluginPermission(const string& pluginName, const string& permissionValue) {
    auto it = registerPerm.find(pluginName);
    if (it != registerPerm.end()) {
        auto& permissions = it->second;
        auto  permIt =
            std::find_if(permissions.begin(), permissions.end(), [&permissionValue](const Authority& authority) {
                return authority.permissionValue == permissionValue;
            });
        if (permIt != permissions.end()) {
            return *permIt;
        }
    }
    return std::nullopt;
}

bool isPermissionRegistration(const string& pluginName, const string& permissionValue) {
    auto it = registerPerm.find(pluginName);
    if (it != registerPerm.end()) {
        auto& permissions = it->second;
        return std::any_of(permissions.begin(), permissions.end(), [&permissionValue](const Authority& authority) {
            return authority.permissionValue == permissionValue;
        });
    }
    return false;
}

bool registerPermission(const string& pluginName, const string& permissionName, const string& permissionValue) {
    if (!perm::PermissionCore::validatePermission(permissionValue)) return false;
    if (isPermissionRegistration(pluginName, permissionValue)) return false;
    registerPerm[pluginName].push_back({permissionName, permissionValue});
    return true;
}

bool unregisterPermission(const string& pluginName, const string& permissionValue) {
    auto it = registerPerm.find(pluginName);
    if (it != registerPerm.end()) {
        auto& permissions = it->second;
        auto  permIt =
            std::remove_if(permissions.begin(), permissions.end(), [&permissionValue](const Authority& authority) {
                return authority.permissionValue == permissionValue;
            });
        if (permIt != permissions.end()) {
            permissions.erase(permIt, permissions.end());
            return true;
        }
    }
    return false;
}

} // namespace perm::registers