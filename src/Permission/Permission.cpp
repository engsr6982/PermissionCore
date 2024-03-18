#include "permission/Permission.h"
#include "db/db.h"
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
    if (data) {
        return true;
    }
    if (!pcore::db::isPluginInit(pluginName)) {
        pcore::db::initPluginData(pluginName);
    }
    auto d = pcore::db::getPluginData(pluginName);
    if (d) {
        data = std::unique_ptr<perm::structs::PluginPermData>(new perm::structs::PluginPermData(*d));
        return true;
    }
    return false;
}

bool PermissionCore::setPermDataToDB() { return pcore::db::setPluginData(pluginName, *data); }

PermissionCore::PermissionCore(string pluginName, bool enablePublicGroups) {
    pluginName         = pluginName;
    enablePublicGroups = enablePublicGroups;
    loadPermDataFromDB();
}

// !管理员接口 ===========================================================================
// 获取所有管理员
const std::vector<string>& PermissionCore::getAllAdmins() { return data->admin; }

// 检查用户是否为管理员
bool PermissionCore::isAdmin(const string& userid) {
    auto& admins = data->admin;
    return std::find(admins.begin(), admins.end(), userid) != admins.end();
}

// 添加管理员
bool PermissionCore::addAdmin(const string& userid) {
    if (isAdmin(userid)) return false;
    data->admin.push_back(userid);
    return setPermDataToDB();
}

// 移除管理员
bool PermissionCore::removeAdmin(const string& userid) {
    if (!isAdmin(userid)) return false;
    auto& adminGroup = data->admin;
    adminGroup.erase(std::remove(adminGroup.begin(), adminGroup.end(), userid), adminGroup.end());
    return setPermDataToDB();
}

//! 用户组接口 ===========================================================================
// 检查名称是否合法 (允许1-16字节，允许中文字母，数字，下划线)
bool PermissionCore::validateName(const string& name) {
    std::regex pattern("^[a-zA-Z0-9_\u4e00-\u9fa5]{1,16}$");
    return std::regex_match(name, pattern);
}

// 检查组是否存在
bool PermissionCore::hasUserGroup(const string& name) {
    auto& userGroup = data->user;
    for (const auto& group : userGroup) {
        if (group.groupName == name) return true;
    }
    return false;
}

// 获取组
const std::optional<perm::structs::GetUserGroupStruct> PermissionCore::getUserGroup(const string& name) {
    auto& userGroup = data->user;
    for (size_t i = 0; i < userGroup.size(); ++i) {
        if (userGroup[i].groupName == name) {
            // 使用构造函数创建a的实例
            return perm::structs::GetUserGroupStruct(i, userGroup[i]);
        }
    }
    return std::nullopt;
}

// 获取所有组
const std::vector<perm::structs::UserGroup>& PermissionCore::getAllUserGroups() { return data->user; }

// 创建组
bool PermissionCore::createUserGroup(const string& name) {
    if (hasUserGroup(name) || !validateName(name)) return false;
    perm::structs::UserGroup gp;
    gp.groupName = name;
    gp.user      = std::vector<string>();
    gp.authority = std::vector<string>();
    data->user.push_back(gp);
    return setPermDataToDB();
}

// 删除组
bool PermissionCore::deleteUserGroup(const string& name) {
    if (!hasUserGroup(name)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false;
    auto index = group->index;
    data->user.erase(data->user.begin() + index);
    return setPermDataToDB();
}

// 重命名组
bool PermissionCore::renameUserGroup(const string& name, const string& newGroupName) {
    if (!hasUserGroup(name) || !validateName(newGroupName)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false;
    auto index                  = group->index;
    data->user[index].groupName = newGroupName;
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
    data->user[index].authority.push_back(authority);
    return setPermDataToDB();
}

// 从组中移除权限
bool PermissionCore::removePermissionToUserGroup(const string& name, const string& authority) {
    if (!hasUserGroup(name) || !hasUserGroupPermission(name, authority)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false; // 检查group是否有值
    auto& authArray = data->user[group->index].authority;
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
    data->user[index].user.push_back(userid);
    return setPermDataToDB();
}

// 从组中移除用户
bool PermissionCore::removeUserToUserGroup(const string& name, const string& userid) {
    if (!hasUserGroup(name) || !isUserInUserGroup(name, userid)) return false;
    auto group = getUserGroup(name);
    if (!group.has_value()) return false; // 检查group是否有值
    auto& userArray = data->user[group->index].user;
    userArray.erase(std::remove(userArray.begin(), userArray.end(), userid), userArray.end());
    return setPermDataToDB();
}

// 获取用户所在的组
const std::vector<perm::structs::UserGroup> PermissionCore::getUserGroupsOfUser(const string& userid) {
    std::vector<perm::structs::UserGroup> us;

    auto& userGroup = data->user;
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
const std::optional<perm::structs::GetUserPermissionsStruct>
PermissionCore::getUserPermissionOfUserData(const string& userid) {
    perm::structs::GetUserPermissionsStruct data;

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
const std::vector<std::string>& PermissionCore::getPublicGroupAllPermissions() { return data->publicAuthority; }

// 检查公共组是否具有特定权限
bool PermissionCore::hasPublicGroupPermission(const string& authority) {
    auto& p = data->publicAuthority;
    return std::find(p.begin(), p.end(), authority) != p.end();
}

// 向公共组添加权限
bool PermissionCore::addPermissionToPublicGroup(const string& authority) {
    if (!validatePermission(authority)) return false;
    if (hasPublicGroupPermission(authority)) return false;
    data->publicAuthority.push_back(authority);
    return setPermDataToDB();
}

// 从公共组中移除权限
bool PermissionCore::removePermissionToPublicGroup(const string& authority) {
    if (!hasPublicGroupPermission(authority)) return false;
    auto& p = data->publicAuthority;
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
    return publicGroup && enablePublicGroups ? this->hasPublicGroupPermission(authority)
         : adminGroup                        ? isAdmin(userid)
                                             : false;
}

//! 权限注册接口  ===========================================================================

// 检查权限是否合法 (6~12个字符，允许数字，字母)
bool PermissionCore::validatePermission(const string& authority) {
    std::regex pattern("^[a-zA-Z0-9]{6,12}$");
    return std::regex_match(authority, pattern);
}

// /**
//  * 获取所有已注册的权限
//  * @returns 权限列表
//  */
// json PermissionCore::retrieveAllPermissions() { return this->registeredPermissions; }

// /**
//  * 获取权限
//  * @param value 权限值
//  * @returns 权限对象
//  */
// std::optional<json> PermissionCore::retrievePermission(const string& value) {
//     auto it = std::find_if(
//         this->registeredPermissions.begin(),
//         this->registeredPermissions.end(),
//         [&value](const json& item) { return item["value"] == value; }
//     );
//     if (it != this->registeredPermissions.end()) {
//         return *it;
//     }
//     return std::nullopt;
// }

// /**
//  * 检查权限是否已注册
//  * @param authority 权限
//  * @returns 注册状态
//  */
// bool PermissionCore::checkPermissionRegistration(const string& authority) {
//     return std::any_of(
//         this->registeredPermissions.begin(),
//         this->registeredPermissions.end(),
//         [&authority](const json& item) { return item["value"] == authority; }
//     );
// }

// /**
//  * 注册权限
//  * @param name 权限名称
//  * @param authority 权限值
//  * @returns 成功状态
//  */
// bool PermissionCore::registerPermission(const string& name, const string& authority) {
//     if (!validatePermission(authority)) return false;         // 权限值无效
//     if (checkPermissionRegistration(authority)) return false; // 权限值已注册
//     this->registeredPermissions.push_back({
//         {"name",  name     },
//         {"value", authority},
//     });
//     return true;
// }

// /**
//  * 注销权限
//  * @param authority 权限
//  * @returns 成功状态
//  */
// bool PermissionCore::unregisterPermission(const string& authority) {
//     if (!checkPermissionRegistration(authority)) return false;
//     auto it = std::remove_if(
//         this->registeredPermissions.begin(),
//         this->registeredPermissions.end(),
//         [&authority](const json& item) { return item["value"] == authority; }
//     );
//     this->registeredPermissions.erase(it, this->registeredPermissions.end());
//     return true;
// }

} // namespace perm
