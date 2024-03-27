#pragma once

#include "Macros.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace perm {

using string = std::string;

struct PermExports UserGroup {
    std::string              groupName;
    std::vector<std::string> authority;
    std::vector<std::string> user;
};
struct PermExports PluginPermData {
    std::vector<std::string> admin;
    std::vector<UserGroup>   user;
    std::vector<std::string> publicAuthority;
};
struct PermExports GetUserGroupStruct {
    const int       index;
    const UserGroup data;

    GetUserGroupStruct(int idx, const UserGroup& grp) : index(idx), data(grp) {}
};
struct PermExports GetUserPermissionsStruct {
    std::unordered_map<string, std::vector<string>> source;
    std::vector<string>                             authority;
};

class PermExports PermissionCore {
public:
    PermissionCore(string pluginName, bool enablePublicGroups);

    //! admin
    const std::vector<string>& getAllAdmins();

    bool isAdmin(const std::string& userid);

    bool addAdmin(const std::string& userid);

    bool removeAdmin(const std::string& userid);

    //! user
    bool hasUserGroup(const std::string& name);

    const std::optional<GetUserGroupStruct> getUserGroup(const string& name);

    const std::vector<UserGroup>& getAllUserGroups();

    bool createUserGroup(const std::string& name);

    bool deleteUserGroup(const std::string& name);

    bool renameUserGroup(const std::string& name, const std::string& newGroupName);

    bool hasUserGroupPermission(const std::string& name, const std::string& authority);

    bool addPermissionToUserGroup(const std::string& name, const std::string& authority);

    bool removePermissionToUserGroup(const std::string& name, const std::string& authority);

    bool isUserInUserGroup(const std::string& name, const std::string& userid);

    bool addUserToUserGroup(const std::string& name, const std::string& userid);

    bool removeUserToUserGroup(const std::string& name, const std::string& userid);

    const std::vector<UserGroup> getUserGroupsOfUser(const string& userid);

    const std::optional<GetUserPermissionsStruct> getUserPermissionOfUserData(const string& userid);

    //! public
    const std::vector<std::string>& getPublicGroupAllPermissions();

    bool hasPublicGroupPermission(const std::string& authority);

    bool addPermissionToPublicGroup(const std::string& authority);

    bool removePermissionToPublicGroup(const std::string& authority);

    //! other
    bool
    checkUserPermission(const string& userid, const string& authority, const bool publicGroup, const bool adminGroup);

    //! tools
    static bool validatePermission(const std::string& authority);
    static bool validateName(const std::string& name);

private:
    std::unique_ptr<PluginPermData> mData;

    bool   mEnablePublicGroups;
    string mPluginName;
    bool   loadPermDataFromDB();
    bool   setPermDataToDB();
};

} // namespace perm