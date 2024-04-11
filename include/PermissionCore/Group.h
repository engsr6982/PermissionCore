#pragma once

#include "Macros.h"
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


namespace perm::group {

using string = std::string;
using json   = nlohmann::json;

enum class GroupStatus { Disabled, Open };
enum class IgnoreListType { None, UserList, PermissionList };

struct PermExports User {
    string realName;
    string uuid;

    User(const string& playerName, const string& playerUuid) : realName(playerName), uuid(playerUuid){};
    bool operator==(const User& other) const { return realName == other.realName && uuid == other.uuid; }
};

struct PermExports Permission {
    string name;
    int    value;

    Permission(const string& permissionName, const int& val) : name(permissionName), value(val){};
    bool operator==(const Permission& other) const { return name == other.name && value == other.value; }
};

struct PermExports Group {
    string                  groupName;
    std::vector<User>       userList;
    std::vector<Permission> permissionList;
    GroupStatus             status;         // only api: checkUserPermission use
    IgnoreListType          ignoreListType; // only api: checkUserPermission use

    Group(const string& name) : groupName(name), status(GroupStatus::Open), ignoreListType(IgnoreListType::None) {
        userList       = std::vector<User>();
        permissionList = std::vector<Permission>();
    }
    Group(const string& name, GroupStatus grpStatus, IgnoreListType ignoreType)
    : groupName(name),
      status(grpStatus),
      ignoreListType(ignoreType) {
        userList       = std::vector<User>();
        permissionList = std::vector<Permission>();
    }

    // 查找用户
    inline const User* findUser(const string& name_uuid) const {
        for (const auto& user : userList) {
            if (user.uuid == name_uuid || user.realName == name_uuid) {
                return &user;
            }
        }
        return nullptr;
    }

    // 根据UUID查找用户
    inline const User* findUserWithUuid(const string& uuid) const {
        for (const auto& user : userList) {
            if (user.uuid == uuid) {
                return &user;
            }
        }
        return nullptr;
    }

    // 根据realName查找用户
    inline const User* findUserWithRealName(const string& realName) const {
        for (const auto& user : userList) {
            if (user.realName == realName) {
                return &user;
            }
        }
        return nullptr;
    }

    // 根据权限值查找权限
    inline const Permission* findPermissionWithValue(const int& value) const {
        for (const auto& perm : permissionList) {
            if (perm.value == value) {
                return &perm;
            }
        }
        return nullptr;
    }

    inline bool hasUser(const string& name_uuid) const { return findUser(name_uuid) != nullptr; }
    inline bool hasPermission(const int& value) const { return findPermissionWithValue(value) != nullptr; }

    // tools
    inline static Group fromJSON(const json& j) {
        string         name       = j["groupName"].get<std::string>();
        GroupStatus    grpStatus  = static_cast<GroupStatus>(j["status"].get<int>());
        IgnoreListType ignoreType = static_cast<IgnoreListType>(j["ignoreListType"].get<int>());

        // 使用新的构造函数创建Group对象
        Group group(name, grpStatus, ignoreType);

        // 用户列表
        if (j.contains("userList") && j["userList"].is_array()) {
            for (const auto& userJson : j["userList"]) {
                User user(userJson["realName"].get<std::string>(), userJson["uuid"].get<std::string>());
                group.userList.push_back(user);
            }
        }

        // 权限列表
        if (j.contains("permissionList") && j["permissionList"].is_array()) {
            for (const auto& permJson : j["permissionList"]) {
                Permission perm(permJson["name"].get<std::string>(), permJson["value"].get<int>());
                group.permissionList.push_back(perm);
            }
        }

        return group;
    }

    inline json toJson() const {
        json jsonGroup;
        jsonGroup["groupName"]      = groupName;
        jsonGroup["status"]         = static_cast<int>(status);
        jsonGroup["ignoreListType"] = static_cast<int>(ignoreListType);

        // 用户列表
        json userListJson = json::array();
        for (const auto& user : userList) {
            json userJson;
            userJson["realName"] = user.realName;
            userJson["uuid"]     = user.uuid;
            userListJson.push_back(userJson);
        }
        jsonGroup["userList"] = userListJson;

        // 权限列表
        json permissionListJson = json::array();
        for (const auto& perm : permissionList) {
            json permJson;
            permJson["name"]  = perm.name;
            permJson["value"] = perm.value;
            permissionListJson.push_back(permJson);
        }
        jsonGroup["permissionList"] = permissionListJson;

        return jsonGroup;
    }

    inline string toString(int indent) const { return toJson().dump(indent); }
};

} // namespace perm::group
