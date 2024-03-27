#include "PermissionCore/PermissionCore.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/Registers.h"
#include <ll/api/data/KeyValueDB.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>


namespace perm::db {

using string = std::string;
using json   = nlohmann::json;

extern std::unique_ptr<ll::data::KeyValueDB> mKVDB;
std::unique_ptr<ll::data::KeyValueDB>&       getInstance();

bool loadLevelDB();

json           to_json(const PluginPermData& permData);
PluginPermData from_json(const json& j);

std::optional<PluginPermData> getPluginData(string pluginName);
bool                          setPluginData(string pluginName, PluginPermData& data);
bool                          initPluginData(string pluginName);
bool                          isPluginInit(string pluginName);

} // namespace perm::db