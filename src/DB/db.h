#include <ll/api/data/KeyValueDB.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>

namespace pcore::db {

using string = std::string;
using json   = nlohmann::json;

extern std::unique_ptr<ll::data::KeyValueDB> mKVDB;
std::unique_ptr<ll::data::KeyValueDB>&       getInstance();

bool loadLevelDB();

struct UserGroup {
    std::string              groupName;
    std::vector<std::string> authority;
    std::vector<std::string> user;
};
struct PermData {
    std::vector<std::string> admin;
    std::vector<UserGroup>   user;
    std::vector<std::string> publicAuthority;
};

json     to_json(const PermData& permData);
PermData from_json(const json& j);

std::optional<PermData> getPluginData(string pluginName);
bool                    setPluginData(string pluginName, PermData& data);
bool                    initPluginData(string pluginName);

} // namespace pcore::db