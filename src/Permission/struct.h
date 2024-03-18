#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace perm::structs {

using string = std::string;

struct UserGroup {
    std::string              groupName;
    std::vector<std::string> authority;
    std::vector<std::string> user;
};
struct PluginPermData {
    std::vector<std::string> admin;
    std::vector<UserGroup>   user;
    std::vector<std::string> publicAuthority;
};
struct GetUserGroupStruct {
    const int                      index;
    const perm::structs::UserGroup data;

    GetUserGroupStruct(int idx, const perm::structs::UserGroup& grp) : index(idx), data(grp) {}
};
struct GetUserPermissionsStruct {
    std::unordered_map<string, std::vector<string>> source;
    std::vector<string>                             authority;
};

} // namespace perm::structs