#pragma once

#include "PermissionCore.h"
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace pmc {

class PermExports PermissionManager {
public:
    static PermissionManager& getInstance();

    bool registerPermissionCore(const std::string& pluginName, std::shared_ptr<PermissionCore> permCore);
    std::shared_ptr<PermissionCore> getPermissionCore(const std::string& pluginName);
    bool                            unregisterPermissionCore(const std::string& pluginName);

    [[deprecated]] bool hasRegisterPermissionCore(const std::string& pluginName);
    bool                hasPermissionCore(const std::string& pluginName);

    [[deprecated]] std::vector<string> getAllKeys();
    std::vector<std::string>           getAllPluginNames();

private:
    PermissionManager()                                    = default;
    ~PermissionManager()                                   = default;
    PermissionManager(const PermissionManager&)            = delete;
    PermissionManager& operator=(const PermissionManager&) = delete;
    //                 pluginName    PermissionCore
    std::unordered_map<std::string, std::shared_ptr<PermissionCore>> permissionCores;
};

PermExports void AutoRegisterCoreToManager(const std::string& pluginName);

} // namespace pmc