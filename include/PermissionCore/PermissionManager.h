#pragma once

#include "PermissionCore.h"
#include <stdexcept>
#include <unordered_map>

namespace perm {

class PermExports PermissionManager {
public:
    static PermissionManager& getInstance();

    bool registerPermissionCore(const std::string& pluginName, std::shared_ptr<PermissionCore> permCore);
    std::shared_ptr<PermissionCore> getPermissionCore(const std::string& pluginName);
    bool                            unregisterPermissionCore(const std::string& pluginName);

    bool hasRegisterPermissionCore(const std::string& pluginName);

private:
    PermissionManager() = default;
    std::unordered_map<std::string, std::shared_ptr<PermissionCore>> permissionCores;
};

PermExports void initPluginWithPermissionCore(const std::string& pluginName, bool enablePublicGroups);

} // namespace perm