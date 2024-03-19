#include "db/db.h"
#include "entry/Macros.h"
#include "permission/struct.h"
#include <memory>
#include <string>
#include <vector>


namespace perm {

using string = std::string;

class PERMISSION_CORE_API PermissionCore {
public:
    PermissionCore(string pluginName, bool enablePublicGroups);

    //! admin
    const std::vector<string>& getAllAdmins();

    bool isAdmin(const std::string& userid);

    bool addAdmin(const std::string& userid);

    bool removeAdmin(const std::string& userid);

    //! user
    bool validateName(const std::string& name);

    bool hasUserGroup(const std::string& name);

    const std::optional<perm::structs::GetUserGroupStruct> getUserGroup(const string& name);

    const std::vector<perm::structs::UserGroup>& getAllUserGroups();

    bool createUserGroup(const std::string& name);

    bool deleteUserGroup(const std::string& name);

    bool renameUserGroup(const std::string& name, const std::string& newGroupName);

    bool hasUserGroupPermission(const std::string& name, const std::string& authority);

    bool addPermissionToUserGroup(const std::string& name, const std::string& authority);

    bool removePermissionToUserGroup(const std::string& name, const std::string& authority);

    bool isUserInUserGroup(const std::string& name, const std::string& userid);

    bool addUserToUserGroup(const std::string& name, const std::string& userid);

    bool removeUserToUserGroup(const std::string& name, const std::string& userid);

    const std::vector<perm::structs::UserGroup> getUserGroupsOfUser(const string& userid);

    const std::optional<perm::structs::GetUserPermissionsStruct> getUserPermissionOfUserData(const string& userid);

    //! public
    const std::vector<std::string>& getPublicGroupAllPermissions();

    bool hasPublicGroupPermission(const std::string& authority);

    bool addPermissionToPublicGroup(const std::string& authority);

    bool removePermissionToPublicGroup(const std::string& authority);

    //! other
    bool
    checkUserPermission(const string& userid, const string& authority, const bool publicGroup, const bool adminGroup);

    //! reg
    bool validatePermission(const std::string& authority);
    // nlohmann::json                retrieveAllPermissions();
    // std::optional<nlohmann::json> retrievePermission(const std::string& value);
    // bool                          checkPermissionRegistration(const std::string& authority);
    // bool                          registerPermission(const std::string& name, const std::string& authority);
    // bool                          unregisterPermission(const std::string& authority);

private:
    std::unique_ptr<perm::structs::PluginPermData> data;

    bool   enablePublicGroups;
    string pluginName;
    bool   loadPermDataFromDB();
    bool   setPermDataToDB();
};

} // namespace perm