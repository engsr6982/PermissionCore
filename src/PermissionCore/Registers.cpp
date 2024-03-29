#include "PermissionCore/Registers.h"
#include "PermissionCore/PermissionCore.h"
#include "PermissionCore/PermissionManager.h"
#include <algorithm>


namespace perm::registers {

std::unordered_map<string, std::vector<Authority>> registerPerm;

std::vector<Authority> getPluginAllPermissions(const string& pluginName) {
    auto it = registerPerm.find(pluginName);
    if (it != registerPerm.end()) {
        return it->second;
    }
    return {}; // 返回一个新的空vector实例
}

std::optional<Authority> getPluginPermission(const string& pluginName, const string& permissionValue) {
    auto it = registerPerm.find(pluginName);
    if (it != registerPerm.end()) {
        auto& permissions = it->second;
        auto  permIt =
            std::find_if(permissions.begin(), permissions.end(), [&permissionValue](const Authority& authority) {
                return authority.permissionValue == permissionValue;
            });
        if (permIt != permissions.end()) {
            return *permIt; // 直接返回找到的Authority包装在std::optional中
        }
    }
    return std::nullopt; // 当找不到时返回std::nullopt
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
    // if (!perm::PermissionCore::validatePermission(permissionValue)) return false;
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