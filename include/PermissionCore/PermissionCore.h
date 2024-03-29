#pragma once

#include "Group.h"
#include "Macros.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace perm {

using string = std::string;

struct PermExports UserPermissionList {
    std::unordered_map<string, std::vector<string>> source;
    std::vector<int>                                value;

    bool hasPermission(int val) { return std::find(value.begin(), value.end(), val) == value.end(); }
};

class PermExports PermissionCore {
public:
    PermissionCore(string pluginName, bool enablePublicGroups);

    bool hasGroup(const string& name);

    const std::optional<group::Group> getGroup(const string& name);

    const std::vector<group::Group> getAllGroups();

    // TODO:
    const std::vector<group::Group> getAllGroupWithOpen();
    const std::vector<group::Group> getAllGroupWithDisabled();

    bool createGroup(const string& name, bool canBeDeleted);

    bool deleteGroup(const string& name);

    bool renameGroup(const string& name, const string& newGroupName);

    bool hasGroupPermission(const string& name, const int& value);

    bool addPermissionToGroup(const string& name, const string& permissionName, const int& value);

    bool removePermissionToGroup(const string& name, const int& value);

    bool isUserInGroup(const string& name, const string& identifier);

    bool addUserToGroup(const string& name, const string& realName, const string& uuid);

    bool removeUserToGroup(const string& name, const string& identifier);

    const std::vector<group::Group> getGroupsOfUser(const string& identifier);

    const std::optional<UserPermissionList> getUserPermission(const string& userid);

    //! other
    bool checkUserPermission(
        const string& userid,
        const int&    value,
        const bool    ignoreGroupStatus    = false,
        const bool    ignoreIgnoreListType = false
    );

    //! tools
    static bool validateName(const string& name);
    bool        trySyncDataToDB() { return setPermDataToDB(); }

private:
    std::unique_ptr<std::unordered_map<string, group::Group>> mData;

    string mPluginName;
    bool   loadPermDataFromDB();
    bool   setPermDataToDB();
};

} // namespace perm