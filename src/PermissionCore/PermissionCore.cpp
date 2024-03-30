#include "PermissionCore/PermissionCore.h"
#include "DB/db.h"
#include "PermissionCore/PermissionManager.h"
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

PermissionCore::PermissionCore(const string pluginName) {
    mPluginName = pluginName;
    loadPermDataFromDB();
}

//! API接口 ===========================================================================
// 检查组是否存在
bool PermissionCore::hasGroup(const string& groupName) { return mData->find(groupName) != mData->end(); }

// 获取组
const std::optional<group::Group> PermissionCore::getGroup(const string& groupName) {
    if (hasGroup(groupName)) {
        return mData->find(groupName)->second;
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
bool PermissionCore::createGroup(const string& groupName) {
    if (hasGroup(groupName) || !validateName(groupName)) return false;
    group::Group gp(groupName);
    mData->emplace(groupName, std::move(gp));
    return setPermDataToDB();
}

// 删除组
bool PermissionCore::deleteGroup(const string& groupName) {
    if (!hasGroup(groupName)) return false;
    mData->erase(groupName);
    return setPermDataToDB();
}

// 重命名组
bool PermissionCore::renameGroup(const string& groupName, const string& newGroupName) {
    if (!hasGroup(groupName) || !validateName(newGroupName)) return false;
    auto groupIt = mData->find(groupName);
    if (groupIt == mData->end()) return false;

    group::Group group = groupIt->second;           // 复制原来的组
    group.groupName    = newGroupName;              // 修改名称
    mData->erase(groupIt);                          // 删除旧的键值对
    mData->emplace(newGroupName, std::move(group)); // 插入新的键值对
    return setPermDataToDB();
}

// 检查组是否具有特定权限
bool PermissionCore::hasGroupPermission(const string& groupName, const int& permissionValue) {
    auto group = getGroup(groupName);
    if (!group.has_value()) return false;
    return group->hasPermission(permissionValue);
}

// 向组添加权限
bool PermissionCore::addPermissionToGroup(
    const string& groupName,
    const string& permissionName,
    const int&    permissionValue
) {
    if (!hasGroup(groupName) || hasGroupPermission(groupName, permissionValue)) return false;
    auto groupIt = mData->find(groupName);
    if (groupIt == mData->end()) return false;

    group::Permission pr(permissionName, permissionValue);
    groupIt->second.permissionList.push_back(pr);
    return setPermDataToDB();
}

// 从组中移除权限
bool PermissionCore::removePermissionToGroup(const string& groupName, const int& permissionValue) {
    if (!hasGroup(groupName) || !hasGroupPermission(groupName, permissionValue)) return false;
    auto groupOpt = mData->find(groupName);
    if (groupOpt == mData->end()) return false;
    auto& group  = *groupOpt;
    auto  permIt = std::find_if(
        group.second.permissionList.begin(),
        group.second.permissionList.end(),
        [permissionValue](const group::Permission& perm) { return perm.value == permissionValue; }
    );
    if (permIt != group.second.permissionList.end()) {
        group.second.permissionList.erase(permIt);
        return setPermDataToDB();
    }
    return false;
}

// 检查组是否有指定用户
bool PermissionCore::isUserInGroup(const string& groupName, const string& name_uuid) {
    auto group = getGroup(groupName);
    if (!group.has_value()) return false;
    return group->hasUser(name_uuid);
}

// 将用户添加到组
bool PermissionCore::addUserToGroup(const string& groupName, const string& realName, const string& uuid) {
    if (!hasGroup(groupName) || isUserInGroup(groupName, realName)) return false;
    auto groupOpt = mData->find(groupName);
    if (groupOpt == mData->end()) return false;
    group::User us(realName, uuid);
    groupOpt->second.userList.push_back(us);
    return setPermDataToDB();
}

// 从组中移除用户
bool PermissionCore::removeUserToGroup(const string& groupName, const string& name_uuid) {
    if (!hasGroup(groupName) || !isUserInGroup(groupName, name_uuid)) return false;
    auto groupOpt = mData->find(groupName);
    if (groupOpt == mData->end()) return false;
    auto& group = *groupOpt;
    auto  userIt =
        std::find_if(group.second.userList.begin(), group.second.userList.end(), [name_uuid](const group::User& user) {
            return user.realName == name_uuid || user.uuid == name_uuid;
        });
    if (userIt != group.second.userList.end()) {
        group.second.userList.erase(userIt);
        return setPermDataToDB();
    }
    return false;
}

// 获取用户所在的组
const std::vector<group::Group> PermissionCore::getGroupsOfUser(const string& name_uuid) {
    std::vector<group::Group> us;

    auto& userGroup = *mData;
    for (const auto& group : userGroup) {
        if (group.second.hasUser(name_uuid)) {
            us.push_back(group.second);
        }
    }
    return us;
}

// 获取用户权限
const std::optional<UserPermissionList> PermissionCore::getUserPermission(const string& name_uuid) {
    UserPermissionList data;

    auto groups = getGroupsOfUser(name_uuid);
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
    const string& name_uuid,
    const int&    permissionValue,
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
                if (group.hasPermission(permissionValue)) {
                    return true;
                }
                break;
            case group::IgnoreListType::PermissionList:
                // 忽略权限列表，只检查用户
                if (group.hasUser(name_uuid)) {
                    return true;
                }
                break;
            case group::IgnoreListType::None:
                // 不忽略，需要同时检查用户和权限
                if (group.hasUser(name_uuid) && group.hasPermission(permissionValue)) {
                    return true;
                }
                break;
            }
        } else {
            // 忽略ignoreListType，只要用户在组中或组有此权限即可
            if (group.hasUser(name_uuid) || group.hasPermission(permissionValue)) {
                return true;
            }
        }
    }

    // 如果没有找到符合条件的组，返回false
    return false;
}

//! 辅助函数  ===========================================================================

// 检查名称是否合法 (允许1-16字节，允许中文字母，数字，下划线)
bool PermissionCore::validateName(const string& groupName) {
    std::regex pattern("^[a-zA-Z0-9_\u4e00-\u9fa5]{1,16}$");
    return std::regex_match(groupName, pattern);
}
bool PermissionCore::trySyncDataToDB() { return setPermDataToDB(); }

} // namespace perm
