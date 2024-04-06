#include "PermissionCore/PermissionManager.h"
#include <vector>

namespace perm {

PermissionManager& PermissionManager::getInstance() {
    static PermissionManager instance;
    return instance;
}

bool PermissionManager::registerPermissionCore(
    const std::string&              pluginName,
    std::shared_ptr<PermissionCore> permCore
) {
    if (pluginName.empty()) {
        return false;
    }
    if (permissionCores.find(pluginName) != permissionCores.end()) {
        return false; // Plugin already registered
    }
    permissionCores[pluginName] = permCore;
    return true;
}

std::shared_ptr<PermissionCore> PermissionManager::getPermissionCore(const std::string& pluginName) {
    auto it = permissionCores.find(pluginName);
    if (it != permissionCores.end()) {
        return it->second;
    }
    return nullptr; // Not found
}

bool PermissionManager::unregisterPermissionCore(const std::string& pluginName) {
    auto it = permissionCores.find(pluginName);
    if (it != permissionCores.end()) {
        permissionCores.erase(it);
        return true;
    }
    return false;
}

bool PermissionManager::hasRegisterPermissionCore(const std::string& pluginName) {
    return permissionCores.find(pluginName) != permissionCores.end();
}

std::vector<string> PermissionManager::getAllKeys() {
    std::vector<string> keys;
    for (const auto& pair : permissionCores) {
        keys.push_back(pair.first);
    }
    return keys;
}

void AutoRegisterCoreToManager(const std::string& pluginName) {
    auto __TryRegisterCore = std::make_shared<PermissionCore>(pluginName);
    bool __TryRegisterToManager =
        PermissionManager::getInstance().registerPermissionCore(pluginName, __TryRegisterCore);
    if (!__TryRegisterToManager) {
        throw std::runtime_error("Plugin registration failed: " + pluginName);
    }
}

} // namespace perm