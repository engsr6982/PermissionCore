#pragma once

#include "Group.h"
#include "Macros.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace pmc {

using string = std::string;

struct PermExports UserPermissionList {
    std::unordered_map<string, std::vector<string>> source;
    std::vector<int>                                value;

    bool hasPermission(int val) { return std::find(value.begin(), value.end(), val) == value.end(); }
};

class PermExports PermissionCore {
public:
    PermissionCore(const string pluginName);

    bool hasGroup(const string& groupName);

    const std::optional<group::Group> getGroup(const string& groupName);
    const std::vector<group::Group>   getAllGroups();
    const std::vector<group::Group>   getAllGroupWithOpen();
    const std::vector<group::Group>   getAllGroupWithDisabled();

    bool createGroup(const string& groupName);
    bool deleteGroup(const string& groupName);
    bool renameGroup(const string& groupName, const string& newGroupName);

    bool hasGroupPermission(const string& groupName, const int& permissionValue);
    bool addPermissionToGroup(const string& groupName, const string& permissionName, const int& permissionValue);
    bool removePermissionToGroup(const string& groupName, const int& permissionValue);

    bool isUserInGroup(const string& groupName, const string& name_uuid);
    bool addUserToGroup(const string& groupName, const string& realName, const string& uuid);
    bool removeUserToGroup(const string& groupName, const string& name_uuid);

    const std::vector<group::Group>         getGroupsOfUser(const string& name_uuid);
    const std::optional<UserPermissionList> getUserPermission(const string& name_uuid);

    //! other
    bool checkUserPermission(
        const string& name_uuid,
        const int&    permissionValue,
        const bool    ignoreGroupStatus    = false,
        const bool    ignoreIgnoreListType = false
    );

    //! tools
    static bool validateName(const string& groupName);
    bool        trySyncDataToDB();

private:
    std::unique_ptr<std::unordered_map<string, group::Group>> mData;

    string mPluginName;
    bool   loadPermDataFromDB();
    bool   setPermDataToDB();
};

} // namespace pmc