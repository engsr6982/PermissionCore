#pragma once

#include "entry/Macros.h"
#include <string>
#include <unordered_map>
#include <vector>


namespace perm::structs {

using string = std::string;

struct PERMISSION_CORE_API UserGroup {
    std::string              groupName;
    std::vector<std::string> authority;
    std::vector<std::string> user;
};
struct PERMISSION_CORE_API PluginPermData {
    std::vector<std::string> admin;
    std::vector<UserGroup>   user;
    std::vector<std::string> publicAuthority;
};
struct PERMISSION_CORE_API GetUserGroupStruct {
    const int                      index;
    const perm::structs::UserGroup data;

    GetUserGroupStruct(int idx, const perm::structs::UserGroup& grp) : index(idx), data(grp) {}
};
struct PERMISSION_CORE_API GetUserPermissionsStruct {
    std::unordered_map<string, std::vector<string>> source;
    std::vector<string>                             authority;
};

} // namespace perm::structs