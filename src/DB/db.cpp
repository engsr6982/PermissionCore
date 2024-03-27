#include "db.h"
#include "PermissionCore/PermissionCore.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/Registers.h"
#include "entry/Entry.h"
#include <ll/api/data/KeyValueDB.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>


namespace perm::db {

std::unique_ptr<ll::data::KeyValueDB>  mKVDB;
std::unique_ptr<ll::data::KeyValueDB>& getInstance() { return mKVDB; }

bool loadLevelDB() {
    auto& mSelf  = entry::entry::getInstance().getSelf();
    auto& logger = mSelf.getLogger();

    try {
        const auto& dirLevelDB = mSelf.getPluginDir() / "LevelDB";
        mKVDB                  = std::make_unique<ll::data::KeyValueDB>(dirLevelDB);
        return true;
    } catch (...) {
        logger.fatal("Failed to load the permission group database");
        return false;
    }
}

using string = std::string;
using json   = nlohmann::json;

PluginPermData from_json(const json& j) {
    PluginPermData permData;
    permData.admin = j["admin"].get<std::vector<std::string>>();
    for (const auto& userGroup : j["user"]) {
        UserGroup group;
        group.groupName = userGroup["groupName"];
        group.authority = userGroup["authority"].get<std::vector<std::string>>();
        group.user      = userGroup["user"].get<std::vector<std::string>>();
        permData.user.push_back(group);
    }
    permData.publicAuthority = j["public"].get<std::vector<std::string>>();
    return permData;
}

json to_json(const PluginPermData& permData) {
    json data;
    data["admin"] = permData.admin;
    for (const auto& group : permData.user) {
        json userGroup;
        userGroup["groupName"] = group.groupName;
        userGroup["authority"] = group.authority;
        userGroup["user"]      = group.user;
        data["user"].push_back(userGroup);
    }
    data["public"] = permData.publicAuthority;
    return data;
}

std::optional<PluginPermData> getPluginData(string pluginName) {
    auto& logger = entry::entry::getInstance().getSelf().getLogger();
    try {
        auto d = mKVDB->get(pluginName);
        if (d && !d->empty()) {
            auto j = json::parse(*d); // 使用 *d 解引用 std::optional 获取 std::string
            return from_json(j);
        }
        return std::nullopt;
    } catch (...) {
        logger.error("Failed to read plugin {} data from the database", pluginName);
        return std::nullopt;
    }
}

bool setPluginData(string pluginName, PluginPermData& data) {
    auto& logger = entry::entry::getInstance().getSelf().getLogger();
    try {
        auto j   = to_json(data);
        auto str = j.dump();
        return mKVDB->set(pluginName, str);
    } catch (...) {
        logger.fatal("Failed to write plugin {} permission data to the database", pluginName);
        return false;
    }
}

bool initPluginData(string pluginName) {
    auto& logger = entry::entry::getInstance().getSelf().getLogger();
    try {
        logger.warn("1");
        auto result = mKVDB->get(pluginName);
        if (result.has_value() && !result->empty()) {
            // 如果result有值且不为空字符串，认为已经初始化，避免重复初始化
            return false;
        }
        logger.warn("2");
        json j;
        j["admin"]  = json::array();
        j["user"]   = json::array();
        j["public"] = json::array();
        auto d      = j.dump();
        return mKVDB->set(pluginName, d);
    } catch (...) {
        logger.error("Failed to initialize plugin {} data", pluginName);
        return false;
    }
}

bool isPluginInit(string pluginName) {
    auto d = mKVDB->get(pluginName);
    if (!d) {
        return false;
    }
    return !d->empty();
}

} // namespace perm::db