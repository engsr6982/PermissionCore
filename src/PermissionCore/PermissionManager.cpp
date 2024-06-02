#include "PermissionCore/PermissionManager.h"
#include "ll/api/i18n/I18n.h"
#include <vector>


namespace pmc {

using ll::i18n_literals ::operator""_tr;

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

bool PermissionManager::hasRegisterPermissionCore(const std::string& pluginName) { // deprecated
    return permissionCores.find(pluginName) != permissionCores.end();
}
bool PermissionManager::hasPermissionCore(const std::string& pluginName) {
    return permissionCores.find(pluginName) != permissionCores.end();
}


std::vector<string> PermissionManager::getAllKeys() { // deprecated
    std::vector<string> keys;
    for (const auto& pair : permissionCores) {
        keys.push_back(pair.first);
    }
    return keys;
}
std::vector<string> PermissionManager::getAllPluginNames() {
    std::vector<string> keys;
    for (const auto& pair : permissionCores) {
        keys.push_back(pair.first);
    }
    return keys;
}

void AutoRegisterCoreToManager(const std::string& pluginName) {
    auto __TryRegisterCore = std::make_shared<PermissionCore>(pluginName);
    bool __TryRegisterToManager =
        PermissionManager::getInstance().registerPermissionCore(std::string(pluginName), __TryRegisterCore);
    if (!__TryRegisterToManager) {
        throw std::runtime_error("注册权限核心到权限管理器失败, 错误插件名: {0}"_tr(pluginName));
    }
}

} // namespace pmc