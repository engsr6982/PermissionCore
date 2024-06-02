#include "db.h"
#include "PermissionCore/PermissionCore.h"
#include "entry/Entry.h"
#include "ll/api/i18n/I18n.h"
#include <ll/api/data/KeyValueDB.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>


namespace pmc {

using string = std::string;
using json   = nlohmann::json;
using ll::i18n_literals ::operator""_tr;

json db::hashMapToJson(const std::unordered_map<std::string, group::Group>& hashMap) {
    json j;
    for (const auto& [key, value] : hashMap) {
        j[key] = value.toJson();
    }
    return j;
}
std::unordered_map<std::string, group::Group> db::jsonToHashMap(const json& j) {
    std::unordered_map<std::string, group::Group> hashMap;
    for (auto& [key, value] : j.items()) {
        group::Group group = group::Group::fromJSON(value);
        hashMap.emplace(key, std::move(group));
    }
    return hashMap;
}

db& db::getInstance() {
    static db instance;
    return instance;
}

void db::loadLevelDB() {
    auto& mSelf  = entry::entry::getInstance().getSelf();
    auto& logger = mSelf.getLogger();
    try {
        const auto& dirLevelDB = mSelf.getPluginDir() / "LevelDB";
        mKVDB                  = std::make_unique<ll::data::KeyValueDB>(dirLevelDB);
    } catch (...) {
        logger.fatal("从LevelDB加载数据失败"_tr());
    }
}
std::unique_ptr<ll::data::KeyValueDB>& db::getDBInstance() { return mKVDB; }

bool db::isPluginInit(string pluginName) {
    auto d = mKVDB->get(pluginName);
    if (!d) {
        return false;
    }
    return !d->empty();
}

bool db::initPluginData(string pluginName) {
    auto& logger = entry::entry::getInstance().getSelf().getLogger();
    try {
        auto result = mKVDB->get(pluginName);
        if (result.has_value() && !result->empty()) {
            return false;
        }
        json j;
        auto d = j.dump();
        return mKVDB->set(pluginName, d);
    } catch (...) {
        logger.error("初始化插件 '{0}' 权限数据失败"_tr(pluginName));
        return false;
    }
}

bool db::setPluginData(string pluginName, const std::unordered_map<std::string, group::Group>& data) {
    auto& logger = entry::entry::getInstance().getSelf().getLogger();
    try {
        auto str = hashMapToJson(data).dump();
        return mKVDB->set(pluginName, str);
    } catch (...) {
        logger.fatal("保存插件 '{0}' 权限数据失败"_tr(pluginName));
        return false;
    }
}

std::optional<std::unordered_map<std::string, group::Group>> db::getPluginData(string pluginName) {
    auto& logger = entry::entry::getInstance().getSelf().getLogger();
    try {
        auto d = mKVDB->get(pluginName);
        if (d && !d->empty()) {
            auto j = json::parse(*d);
            return jsonToHashMap(j);
        }
        return std::nullopt;
    } catch (...) {
        logger.error("读取插件 '{0}' 权限数据失败"_tr(pluginName));
        return std::nullopt;
    }
}

} // namespace pmc