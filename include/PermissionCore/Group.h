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

enum class GroupStatus { Normal, Disabled };
enum class IgnoreListType { UserList, PermissionList };

struct PermExports User {
    string realName;
    string uuid;
};

struct PermExports Permission {
    string name;
    int    value;
    string description; // 可选
};

struct PermExports Group {
    string                  groupName;
    std::vector<User>       userList;
    std::vector<Permission> permissionList;
    bool                    isManagerGroup;
    bool                    isPublicGroup;
    bool                    canBeDeleted;
    GroupStatus             status;
    IgnoreListType          ignoreListType;

    // 查找用户
    const User* findUser(const string& identifier) const {
        for (const auto& user : userList) {
            if (user.uuid == identifier || user.realName == identifier) {
                return &user;
            }
        }
        return nullptr;
    }

    // 根据UUID查找用户
    const User* findUserWithUuid(const string& uuid) const {
        for (const auto& user : userList) {
            if (user.uuid == uuid) {
                return &user;
            }
        }
        return nullptr;
    }

    // 根据realName查找用户
    const User* findUserWithRealName(const string& realName) const {
        for (const auto& user : userList) {
            if (user.realName == realName) {
                return &user;
            }
        }
        return nullptr;
    }

    // 查找用户
    const Permission* findPermission(const string& identifier) const {
        for (const auto& perm : permissionList) {
            if (perm.name == identifier || perm.value == std::stoi(identifier)) {
                return &perm;
            }
        }
        return nullptr;
    }

    // 根据权限名查找权限
    const Permission* findPermissionWithName(const string& name) const {
        for (const auto& perm : permissionList) {
            if (perm.name == name) {
                return &perm;
            }
        }
        return nullptr;
    }

    // 根据权限值查找权限
    const Permission* findPermissionWithValue(int value) const {
        for (const auto& perm : permissionList) {
            if (perm.value == value) {
                return &perm;
            }
        }
        return nullptr;
    }

    bool hasUser(const string& identifier) const { return findUser(identifier) != nullptr; }
    bool hasPermission(const string& identifier) const { return findPermission(identifier) != nullptr; }

    // tools
    static Group fromJSON(const json& j) {
        Group group;
        group.groupName      = j["groupName"].get<std::string>();
        group.isManagerGroup = j["isManagerGroup"].get<bool>();
        group.isPublicGroup  = j["isPublicGroup"].get<bool>();
        group.canBeDeleted   = j["canBeDeleted"].get<bool>();
        group.status         = static_cast<GroupStatus>(j["status"].get<int>());
        group.ignoreListType = static_cast<IgnoreListType>(j["ignoreListType"].get<int>());

        // 用户列表
        if (j.contains("userList") && j["userList"].is_array()) {
            for (const auto& userJson : j["userList"]) {
                User user;
                user.realName = userJson["realName"].get<std::string>();
                user.uuid     = userJson["uuid"].get<std::string>();
                group.userList.push_back(user);
            }
        }

        // 权限列表
        if (j.contains("permissionList") && j["permissionList"].is_array()) {
            for (const auto& permJson : j["permissionList"]) {
                Permission perm;
                perm.name  = permJson["name"].get<std::string>();
                perm.value = permJson["value"].get<int>();
                if (permJson.contains("description")) {
                    perm.description = permJson["description"].get<std::string>();
                }
                group.permissionList.push_back(perm);
            }
        }

        return group;
    }

    json toJson() const {
        json jsonGroup;
        jsonGroup["groupName"]      = groupName;
        jsonGroup["isManagerGroup"] = isManagerGroup;
        jsonGroup["isPublicGroup"]  = isPublicGroup;
        jsonGroup["canBeDeleted"]   = canBeDeleted;
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
            permJson["name"]        = perm.name;
            permJson["value"]       = perm.value;
            permJson["description"] = perm.description; // 可选字段
            permissionListJson.push_back(permJson);
        }
        jsonGroup["permissionList"] = permissionListJson;

        return jsonGroup;
    }

    string toString(int indent) const { return toJson().dump(indent); }
};

} // namespace perm::group
