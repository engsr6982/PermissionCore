#include "PermissionCore/PermissionCore.h"
#include "DB/db.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/Registers.h"
#include "entry/Entry.h"
#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>


namespace perm {

using json   = nlohmann::json;
using string = std::string;

// tools
bool PermissionCore::loadPermDataFromDB() {
    auto& logger = entry::entry::getInstance().getSelf().getLogger();
    if (mData != nullptr) {
        logger.warn("Do not repeat the initialization");
        return true;
    }
    if (!perm::db::getInstance().isPluginInit(mPluginName)) {
        perm::db::getInstance().initPluginData(mPluginName);
    }
    auto d = perm::db::getInstance().getPluginData(mPluginName);
    if (d) {
        mData = std::unique_ptr<std::unordered_map<std::string, group::Group>>(
            new std::unordered_map<std::string, group::Group>(*d)
        );
        logger.info("Initialization of plugin {} permission data is successful", mPluginName);
        return true;
    }
    logger.fatal("Failed to initialize the [{}] permission data of the plugin", mPluginName);
    return false;
}

bool PermissionCore::setPermDataToDB() { return perm::db::getInstance().setPluginData(mPluginName, *mData); }

PermissionCore::PermissionCore(string pluginName) {
    mPluginName = pluginName;
    loadPermDataFromDB();
}

//! API接口 ===========================================================================
// 检查组是否存在
bool PermissionCore::hasGroup(const string& name) { return mData->find(name) != mData->end(); }

// 获取组
const std::optional<group::Group> PermissionCore::getGroup(const string& name) {
    if (hasGroup(name)) {
        return mData->find(name)->second;
    }
    return std::nullopt;
}

// 获取所有组
const std::vector<group::Group> PermissionCore::getAllGroups() {
    std::vector<group::Group> li;
    for (const auto& kv : *mData) {
        li.push_back(kv.second);
    }
    return li;
}

const std::vector<group::Group> PermissionCore::getAllGroupWithDisabled() {
    auto allGroups = getAllGroups();
    auto endIt     = std::remove_if(allGroups.begin(), allGroups.end(), [](const group::Group& gr) {
        return gr.status != group::GroupStatus::Disabled;
    });
    allGroups.erase(endIt, allGroups.end());
    return allGroups;
}

const std::vector<group::Group> PermissionCore::getAllGroupWithOpen() {
    auto allGroups = getAllGroups();
    auto endIt     = std::remove_if(allGroups.begin(), allGroups.end(), [](const group::Group& gr) {
        return gr.status != group::GroupStatus::Open;
    });
    allGroups.erase(endIt, allGroups.end());
    return allGroups;
}

// 创建组
bool PermissionCore::createGroup(const string& name, bool canBeDeleted) {
    if (hasGroup(name) || !validateName(name)) return false;
    group::Group gp(name, canBeDeleted);
    (*mData)[name] = gp;
    return setPermDataToDB();
}

// 删除组
bool PermissionCore::deleteGroup(const string& name) {
    if (!hasGroup(name)) return false;
    mData->erase(name);
    return setPermDataToDB();
}

// 重命名组
bool PermissionCore::renameGroup(const string& name, const string& newGroupName) {
    if (!hasGroup(name) || !validateName(newGroupName)) return false;
    auto group = getGroup(name);
    if (!group.has_value()) return false;
    (*mData)[group->groupName].groupName = newGroupName;
    return setPermDataToDB();
}

// 检查组是否具有特定权限
bool PermissionCore::hasGroupPermission(const string& name, const int& value) {
    auto group = getGroup(name);
    if (!group.has_value()) return false;
    return group->hasPermission(value);
}

// 向组添加权限
bool PermissionCore::addPermissionToGroup(const string& name, const string& permissionName, const int& value) {
    if (!hasGroup(name) || hasGroupPermission(name, value)) return false;
    auto group = getGroup(name);
    if (!group.has_value()) return false;
    group::Permission pr(permissionName, value);
    (*mData)[group->groupName].permissionList.push_back(pr);
    return setPermDataToDB();
}

// 从组中移除权限
bool PermissionCore::removePermissionToGroup(const string& name, const int& value) {
    if (!hasGroup(name) || !hasGroupPermission(name, value)) return false;
    auto group = getGroup(name);
    if (!group.has_value()) return false;
    auto  pm        = const_cast<group::Permission&>(*group->findPermissionWithValue(value));
    auto& authArray = (*mData)[group->groupName].permissionList;
    authArray.erase(std::remove(authArray.begin(), authArray.end(), pm), authArray.end());
    return setPermDataToDB();
}

// 检查组是否有指定用户
bool PermissionCore::isUserInGroup(const string& name, const string& identifier) {
    auto group = getGroup(name);
    if (!group.has_value()) return false;
    return group->hasUser(identifier);
}

// 将用户添加到组
bool PermissionCore::addUserToGroup(const string& name, const string& realName, const string& uuid) {
    if (!hasGroup(name) || isUserInGroup(name, realName)) return false;
    auto group = getGroup(name);
    if (!group.has_value()) return false;
    group::User us(realName, uuid);
    (*mData)[group->groupName].userList.push_back(us);
    return setPermDataToDB();
}

// 从组中移除用户
bool PermissionCore::removeUserToGroup(const string& name, const string& identifier) {
    if (!hasGroup(name) || !isUserInGroup(name, identifier)) return false;
    auto group = getGroup(name);
    if (!group.has_value()) return false;
    auto  us        = const_cast<group::User&>(*group->findUser(identifier));
    auto& userArray = (*mData)[group->groupName].userList;
    userArray.erase(std::remove(userArray.begin(), userArray.end(), us), userArray.end());
    return setPermDataToDB();
}

// 获取用户所在的组
const std::vector<group::Group> PermissionCore::getGroupsOfUser(const string& identifier) {
    std::vector<group::Group> us;

    auto& userGroup = *mData;
    for (const auto& group : userGroup) {
        if (group.second.hasUser(identifier)) {
            us.push_back(group.second);
        }
    }
    return us;
}

// 获取用户权限
const std::optional<UserPermissionList> PermissionCore::getUserPermission(const string& userid) {
    UserPermissionList data;

    auto groups = getGroupsOfUser(userid);
    for (const auto& group : groups) {
        if (group.userList.empty() || group.permissionList.empty()) continue;
        for (const auto& perm : group.permissionList) {
            // 如果权限不在用户权限列表中，则添加
            if (data.hasPermission(perm.value)) {
                data.value.push_back(perm.value);
            }
            // 如果权限来源中没有该权限，则初始化
            if (data.source.find(group.groupName) == data.source.end()) {
                data.source[group.groupName] = std::vector<string>(); // 初始化为空字符串向量
            }
            // 如果权限来源中没有该组名，则添加
            if (std::find(data.source[group.groupName].begin(), data.source[group.groupName].end(), group.groupName)
                == data.source[group.groupName].end()) {
                data.source[group.groupName].push_back(group.groupName);
            }
        }
    }
    return data;
}

//! 其他接口  ===========================================================================

// 检查用户是否具有特定权限
bool PermissionCore::checkUserPermission(
    const string& userid,
    const int&    value,
    const bool    ignoreGroupStatus,
    const bool    ignoreIgnoreListType
) {
    std::vector<group::Group> groups;
    if (ignoreGroupStatus) {
        // 如果忽略组状态，获取所有组
        groups = getAllGroups();
    } else {
        // 否则，只获取状态为Open的组
        groups = getAllGroupWithOpen();
    }

    for (const auto& group : groups) {
        // 根据ignoreIgnoreListType检查忽略情况
        if (!ignoreIgnoreListType) {
            // 不忽略任何情况，需要根据组的ignoreListType来决定如何检查
            switch (group.ignoreListType) {
            case group::IgnoreListType::UserList:
                // 忽略用户列表，只检查权限
                if (group.hasPermission(value)) {
                    return true;
                }
                break;
            case group::IgnoreListType::PermissionList:
                // 忽略权限列表，只检查用户
                if (group.hasUser(userid)) {
                    return true;
                }
                break;
            case group::IgnoreListType::None:
                // 不忽略，需要同时检查用户和权限
                if (group.hasUser(userid) && group.hasPermission(value)) {
                    return true;
                }
                break;
            }
        } else {
            // 忽略ignoreListType，只要用户在组中或组有此权限即可
            if (group.hasUser(userid) || group.hasPermission(value)) {
                return true;
            }
        }
    }

    // 如果没有找到符合条件的组，返回false
    return false;
}

//! 辅助函数  ===========================================================================

// 检查名称是否合法 (允许1-16字节，允许中文字母，数字，下划线)
bool PermissionCore::validateName(const string& name) {
    std::regex pattern("^[a-zA-Z0-9_\u4e00-\u9fa5]{1,16}$");
    return std::regex_match(name, pattern);
}

} // namespace perm
