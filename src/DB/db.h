#pragma once

#include "PermissionCore/Group.h"
#include "PermissionCore/PermissionCore.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/Registers.h"
#include <ll/api/data/KeyValueDB.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>


namespace perm {

using string = std::string;
using json   = nlohmann::json;

class db {
public:
    static db& getInstance();

    void                                   loadLevelDB();
    std::unique_ptr<ll::data::KeyValueDB>& getDBInstance();

    bool isPluginInit(string pluginName);
    bool initPluginData(string pluginName);

    std::optional<std::unordered_map<std::string, group::Group>> getPluginData(string pluginName);
    bool setPluginData(string pluginName, const std::unordered_map<std::string, group::Group>& data);

    // tools
    static json hashMapToJson(const std::unordered_map<std::string, group::Group>& hashMap);
    static std::unordered_map<std::string, group::Group> jsonToHashMap(const json& j);

private:
    db() = default;
    std::unique_ptr<ll::data::KeyValueDB> mKVDB;
};

} // namespace perm