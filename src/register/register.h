#include "entry/Macros.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace perm::registers {

using string = std::string;

struct Authority {
    string permissionName;
    string permissionValue;
};

extern std::unordered_map<string, std::vector<Authority>> registerPerm;

PERMISSION_CORE_API const std::vector<Authority>& getPluginAllPermissions(const string& pluginName);

PERMISSION_CORE_API const std::optional<Authority>&
                          getPluginPermission(const string& pluginName, const string& permissionValue);

PERMISSION_CORE_API bool isPermissionRegistration(const string& pluginName, const string& permissionValue);

PERMISSION_CORE_API bool
registerPermission(const string& pluginName, const string& permissionName, const string& permissionValue);

PERMISSION_CORE_API bool unregisterPermission(const string& pluginName, const string& permissionValue);


} // namespace perm::registers