#include "PermissionCore/PermissionCore.h"
#include "DB/db.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/Registers.h"
#include "entry/Entry.h"
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
    if (!perm::db::isPluginInit(mPluginName)) {
        perm::db::initPluginData(mPluginName);
    }
    auto d = perm::db::getPluginData(mPluginName);
    if (d) {
        mData = std::unique_ptr<PluginPermData>(new PluginPermData(*d));
        logger.info("Initialization of plugin {} permission data is successful", mPluginName);
        return true;
    }
    logger.fatal("Failed to initialize the [{}] permission data of the plugin", mPluginName);
    return false;
}

bool PermissionCore::setPermDataToDB() { return perm::db::setPluginData(mPluginName, *mData); }

PermissionCore::PermissionCore(string pluginName, bool enablePublicGroups) {
    mPluginName         = pluginName;
    mEnablePublicGroups = enablePublicGroups;
    loadPermDataFromDB();
}

// !管理员接口 ===========================================================================
// 获取所有管理员
const std::vector<string>& PermissionCore::getAllAdmins() { return mData->admin; }

// 检查用户是否为管理员
bool PermissionCore::isAdmin(const string& userid) {
    auto& admins = mData->admin;
    return std::find(admins.begin(), admins.end(), userid) != admins.end();
}

// 添加管理员
bool PermissionCore::addAdmin(const string& userid) {
    if (isAdmin(userid)) return false;
    mData->admin.push_back(userid);
    return setPermDataToDB();
}

// 移除管理员
bool PermissionCore::removeAdmin(const string& userid) {
    if (!isAdmin(userid)) return false;
    auto& adminGroup = mData->admin;
    adminGroup.erase(std::remove(adminGroup.begin(), adminGroup.end(), userid), adminGroup.end());
    return setPermDataToDB();
}

//! 用户组接口 ===========================================================================
// 检查组是否存在
bool PermissionCore::hasUserGroup(const string& name) {
    auto& userGroup = mData->user;
    for (const auto& group : userGroup) {
        if (group.groupName == name) return true;
    }
    return false;
}

// 获取组
const std::optional<GetUserGroupStruct> PermissionCore::getUserGroup(const string& name) {
    auto& userGroup = mData->user;
    for (size_t i = 0; i < userGroup.size(); ++i) {
        if (userGroup[i].groupName == name) {
            // 使用构造函数创建a的实例
            return GetUserGroupStruct(i, userGroup[i]);
        }
    }
    return std::nullopt;
}

// 获取所有组
const std::vector<UserGroup>& PermissionCore::getAllUserGroups() { return mData->user; }

// 创建组
bool PermissionCore::createUserGroup(const string& name) {
    if (hasUserGroup(name) || !validateName(name)) return false;
    UserGroup gp;
    gp.groupName = name;
    gp.user      = std::vector<string>();
    gp.authority = std::vector<string>();
    mData->user.push_back(gp);
    return setPermDataToDB();
}

// 删除组
bool PermissionCore::deleteUserGroup(const string& name) {
    if (!hasUserGroup(name)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false;
    auto index = group->index;
    mData->user.erase(mData->user.begin() + index);
    return setPermDataToDB();
}

// 重命名组
bool PermissionCore::renameUserGroup(const string& name, const string& newGroupName) {
    if (!hasUserGroup(name) || !validateName(newGroupName)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false;
    auto index                   = group->index;
    mData->user[index].groupName = newGroupName;
    return setPermDataToDB();
}

// 检查组是否具有特定权限
bool PermissionCore::hasUserGroupPermission(const string& name, const string& authority) {
    auto group = getUserGroup(name);
    if (!group.has_value()) return false;
    for (const auto& auth : group->data.authority) {
        if (auth == authority) return true;
    }
    return false;
}

// 向组添加权限
bool PermissionCore::addPermissionToUserGroup(const string& name, const string& authority) {
    if (!hasUserGroup(name) || !validatePermission(authority) || hasUserGroupPermission(name, authority)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false;
    int index = group->index;
    mData->user[index].authority.push_back(authority);
    return setPermDataToDB();
}

// 从组中移除权限
bool PermissionCore::removePermissionToUserGroup(const string& name, const string& authority) {
    if (!hasUserGroup(name) || !hasUserGroupPermission(name, authority)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false; // 检查group是否有值
    auto& authArray = mData->user[group->index].authority;
    authArray.erase(std::remove(authArray.begin(), authArray.end(), authority), authArray.end());
    return setPermDataToDB();
}

// 检查组是否有指定用户
bool PermissionCore::isUserInUserGroup(const string& name, const string& userid) {
    auto group = getUserGroup(name);
    if (!group.has_value()) return false; // 检查group是否有值
    for (const auto& user : group->data.user) {
        if (user == userid) return true;
    }
    return false;
}

// 将用户添加到组
bool PermissionCore::addUserToUserGroup(const string& name, const string& userid) {
    if (!hasUserGroup(name) || isUserInUserGroup(name, userid)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false;
    auto index = group->index;
    mData->user[index].user.push_back(userid);
    return setPermDataToDB();
}

// 从组中移除用户
bool PermissionCore::removeUserToUserGroup(const string& name, const string& userid) {
    if (!hasUserGroup(name) || !isUserInUserGroup(name, userid)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false; // 检查group是否有值
    auto& userArray = mData->user[group->index].user;
    userArray.erase(std::remove(userArray.begin(), userArray.end(), userid), userArray.end());
    return setPermDataToDB();
}

// 获取用户所在的组
const std::vector<UserGroup> PermissionCore::getUserGroupsOfUser(const string& userid) {
    std::vector<UserGroup> us;

    auto& userGroup = mData->user;
    for (const auto& group : userGroup) {
        for (const auto& user : group.user) {
            if (user == userid) {
                us.push_back(group);
                break;
            }
        }
    }
    return us;
}

// 获取用户权限
const std::optional<GetUserPermissionsStruct> PermissionCore::getUserPermissionOfUserData(const string& userid) {
    GetUserPermissionsStruct data;

    auto groups = getUserGroupsOfUser(userid);
    for (const auto& group : groups) {
        if (group.authority.empty()) continue;
        for (const auto& perm : group.authority) {
            // 如果权限不在用户权限列表中，则添加
            if (std::find(data.authority.begin(), data.authority.end(), perm) == data.authority.end()) {
                data.authority.push_back(perm);
            }
            // 如果权限来源中没有该权限，则初始化
            if (data.source.find(perm) == data.source.end()) {
                data.source[perm] = std::vector<string>(); // 初始化为空字符串向量
            }
            // 如果权限来源中没有该组名，则添加
            if (std::find(data.source[perm].begin(), data.source[perm].end(), group.groupName)
                == data.source[perm].end()) {
                data.source[perm].push_back(group.groupName);
            }
        }
    }
    return data;
}

//! 公共组接口  ===========================================================================
// 获取公共组权限
const std::vector<std::string>& PermissionCore::getPublicGroupAllPermissions() { return mData->publicAuthority; }

// 检查公共组是否具有特定权限
bool PermissionCore::hasPublicGroupPermission(const string& authority) {
    auto& p = mData->publicAuthority;
    return std::find(p.begin(), p.end(), authority) != p.end();
}

// 向公共组添加权限
bool PermissionCore::addPermissionToPublicGroup(const string& authority) {
    if (!validatePermission(authority)) return false;
    if (hasPublicGroupPermission(authority)) return false;
    mData->publicAuthority.push_back(authority);
    return setPermDataToDB();
}

// 从公共组中移除权限
bool PermissionCore::removePermissionToPublicGroup(const string& authority) {
    if (!hasPublicGroupPermission(authority)) return false;
    auto& p = mData->publicAuthority;
    p.erase(std::remove(p.begin(), p.end(), authority), p.end());
    return setPermDataToDB();
}

//! 其他接口  ===========================================================================

// 检查用户是否具有特定权限
bool PermissionCore::checkUserPermission(
    const string& userid,
    const string& authority,
    const bool    publicGroup,
    const bool    adminGroup
) {
    auto userPermissions = this->getUserPermissionOfUserData(userid)->authority;
    if (std::find(userPermissions.begin(), userPermissions.end(), authority) != userPermissions.end()) {
        return true;
    }
    return publicGroup && mEnablePublicGroups ? this->hasPublicGroupPermission(authority)
         : adminGroup                         ? isAdmin(userid)
                                              : false;
}

//! 辅助函数  ===========================================================================

// 检查权限是否合法 (6~12个字符，允许数字，字母)
bool PermissionCore::validatePermission(const string& authority) {
    std::regex pattern("^[a-zA-Z0-9]{6,12}$");
    return std::regex_match(authority, pattern);
}
// 检查名称是否合法 (允许1-16字节，允许中文字母，数字，下划线)
bool PermissionCore::validateName(const string& name) {
    std::regex pattern("^[a-zA-Z0-9_\u4e00-\u9fa5]{1,16}$");
    return std::regex_match(name, pattern);
}

} // namespace perm
