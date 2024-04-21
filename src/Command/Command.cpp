#include "Command.h"

namespace perm::command {


void noPermission(CommandOutput& output) { output.error("This command is available to [OP] only!"); }

using string             = std::string;
using PermissionManager  = perm::PermissionManager;
using PermissionCore     = perm::PermissionCore;
using PermissionRegister = perm::PermissionRegister;

enum OperationAddOrDel : int { add = 0, del = 1 };
enum OperationUserOrPerm : int { user = 0, perm = 1 };

struct UserParmamWithTarget {
    OperationAddOrDel       add_del;
    string                  pluginName;
    string                  groupName;
    CommandSelector<Player> player;
};
struct UserParamWithString {
    OperationAddOrDel add_del;
    string            pluginName;
    string            groupName;
    string            realName;
};

struct PermParamWithString {
    OperationAddOrDel add_del;
    string            pluginName;
    string            groupName;
    int               permValue;
};

struct GroupParamWithString {
    OperationAddOrDel add_del;
    string            pluginName;
    string            groupName;
};

struct ListGroups {
    string pluginName;
    string groupName;
};

void registerCommand() {
    auto& cmd  = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("permc");
    auto  core = perm::PermissionManager::getInstance().getPermissionCore("PermissionCore");

    // permc 打开Gui
    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        Actor* entity = origin.getEntity();
        if (entity) {
            auto& player = *static_cast<Player*>(entity); // entity* => Player&
            if (player.isOperator()) perm::form::index(player);
            else noPermission(output);
        }
    }>();

    // permc list perm <pluginName> [value] 列出插件所有权限值、列出插件的权限详细信息

    // permc list group <pluginName> [groupName] 列出插件所有的组、列出插件的组详细信息

    // permc list plugin [pluginName] 列出所有插件、列出插件详细信息

    // permc group <create|delete> <pluginName> <groupName> 添加、删除组

    // permc user <add|del> <pluginName> <groupName> <target Player> 添加、删除用户到组

    // permc user <add|del> <pluginName> <groupName> <string realName> 添加、删除用户到组

    // permc perm <add|del> <pluginName> <groupName> <value> 添加、删除权限到组


    /////////////////////////////////////////////////////////////////////////////////////////////////////

    // permc user <add|del> <string pluginName> <string groupName> <target Player>
    cmd.overload<UserParmamWithTarget>()
        .text("user")
        .required("add_del")
        .required("pluginName")
        .required("groupName")
        .required("player")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, UserParmamWithTarget const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::DedicatedServer,
                CommandOriginType::Player
            );
            // check is operator
            if (origin.getOriginType() == CommandOriginType::Player) {
                auto& player = *static_cast<Player*>(origin.getEntity());
                if (!player.isOperator()) return noPermission(output);
            }
            // processing...
            PermissionManager& manager = PermissionManager::getInstance();
            if (manager.hasRegisterPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);

                if (core.hasGroup(param.groupName)) {
                    auto target = param.player.results(origin).data;
                    // operation select player
                    for (Player* pl : *target) {
                        if (pl) {
                            string uuid = pl->getUuid().asString().c_str();
                            switch (param.add_del) {
                            case OperationAddOrDel::add: {
                                bool status = core.addUserToGroup(param.groupName, pl->getRealName(), uuid);
                                if (status) {
                                    output.success(
                                        "Add user '{}' to group '{}' in plugin '{}' state '{}'",
                                        pl->getRealName(),
                                        param.groupName,
                                        param.pluginName,
                                        status ? "Success" : "Fail"
                                    );
                                } else {
                                    output.error(
                                        "Add user '{}' to group '{}' in plugin '{}' state '{}'",
                                        pl->getRealName(),
                                        param.groupName,
                                        param.pluginName,
                                        status ? "Success" : "Fail"
                                    );
                                }
                            } break;
                            case OperationAddOrDel::del: {
                                bool status = core.removeUserToGroup(param.groupName, uuid);
                                if (status) {
                                    output.success(
                                        "Remove user '{}' to group '{}' in plugin '{}' state '{}'",
                                        pl->getRealName(),
                                        param.groupName,
                                        param.pluginName,
                                        status ? "Success" : "Fail"
                                    );
                                } else {
                                    output.error(
                                        "Remove user '{}' to group '{}' in plugin '{}' state '{}'",
                                        pl->getRealName(),
                                        param.groupName,
                                        param.pluginName,
                                        status ? "Success" : "Fail"
                                    );
                                }
                            } break;
                            }
                        }
                    }
                } else {
                    output.error("The group '{}' is not registered", param.groupName);
                }
            } else {
                output.error("The target plugin '{}' is not registered", param.pluginName);
            }
        }>();

    // permc user <add|del> <string pluginName> <string groupName> <string realName>
    cmd.overload<UserParamWithString>()
        .text("user")
        .required("add_del")
        .required("pluginName")
        .required("groupName")
        .required("realName")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, UserParamWithString const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::DedicatedServer,
                CommandOriginType::Player
            );
            // check is operator
            if (origin.getOriginType() == CommandOriginType::Player) {
                auto& player = *static_cast<Player*>(origin.getEntity());
                if (!player.isOperator()) return noPermission(output);
            }
            // processing...
            PermissionManager& manager = PermissionManager::getInstance();
            if (manager.hasRegisterPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);
                if (core.hasGroup(param.groupName)) {
                    auto pl = ll::service::getLevel()->getPlayer(param.realName);
                    if (pl) {
                        switch (param.add_del) {
                        case OperationAddOrDel::add: {
                            bool status =
                                core.addUserToGroup(param.groupName, param.realName, pl->getUuid().asString().c_str());
                            if (status) {
                                output.success(
                                    "Add user '{}' to group '{}' in plugin '{}' state '{}'",
                                    param.realName,
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success" : "Fail"
                                );
                            } else {
                                output.error(
                                    "Add user '{}' to group '{}' in plugin '{}' state '{}'",
                                    param.realName,
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success" : "Fail"
                                );
                            }
                        } break;
                        case OperationAddOrDel::del: {
                            bool status = core.removeUserToGroup(param.groupName, pl->getUuid().asString().c_str());
                            if (status) {
                                output.success(
                                    "Remove user '{}' to group '{}' in plugin '{}' state '{}'",
                                    param.realName,
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success" : "Fail"
                                );
                            } else {
                                output.error(
                                    "Remove user '{}' to group '{}' in plugin '{}' state '{}'",
                                    param.realName,
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success" : "Fail"
                                );
                            }
                        } break;
                        }
                    } else {
                        output.error("The player '{}' is not found", param.realName);
                    }
                } else {
                    output.error("The group '{}' is not registered", param.groupName);
                }
            } else {
                output.error("The target plugin '{}' is not registered", param.pluginName);
            }
        }>();

    // permc perm <add|del> <string pluginName> <string groupName> <int permValue>
    cmd.overload<PermParamWithString>()
        .text("perm")
        .required("add_del")
        .required("pluginName")
        .required("groupName")
        .required("permValue")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, PermParamWithString const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::DedicatedServer,
                CommandOriginType::Player
            );
            // check is operator
            if (origin.getOriginType() == CommandOriginType::Player) {
                auto& player = *static_cast<Player*>(origin.getEntity());
                if (!player.isOperator()) return noPermission(output);
            }
            // processing...
            PermissionManager& manager = PermissionManager::getInstance();
            if (manager.hasRegisterPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);
                if (core.hasGroup(param.groupName)) {
                    PermissionRegister registerInst = PermissionRegister::getInstance();
                    if (registerInst.hasPermissionRegisted(param.pluginName, param.permValue)) {
                        auto perm = registerInst.getPermission(param.pluginName, param.permValue);
                        switch (param.add_del) {
                        case OperationAddOrDel::add: {
                            bool status = core.addPermissionToGroup(param.groupName, perm->name, perm->value);
                            if (status) {
                                output.success(
                                    "Add permission '{}' to group '{}' in plugin '{}' state '{}'",
                                    perm->name,
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success" : "Fail"
                                );
                            } else {
                                output.error(
                                    "Add permission '{}' to group '{}' in plugin '{}' state '{}'",
                                    perm->name,
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success" : "Fail"
                                );
                            }
                        } break;
                        case OperationAddOrDel::del: {
                            bool status = core.removePermissionToGroup(param.groupName, perm->value);
                            if (status) {
                                output.success(
                                    "Remove permission '{}' to group '{}' in plugin '{}' state '{}'",
                                    perm->name,
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success" : "Fail"
                                );
                            } else {
                                output.error(
                                    "Remove permission '{}' to group '{}' in plugin '{}' state '{}'",
                                    perm->name,
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success" : "Fail"
                                );
                            }
                        }
                        }
                    } else {
                        output.error("The permission '{}' is not registered", param.permValue);
                    }
                } else {
                    output.error("The group '{}' is not registered", param.groupName);
                }
            } else {
                output.error("The plugin '{}' is not registered", param.pluginName);
            }
        }>();

    // permc group <add|del> <string pluginName> <string groupName>
    cmd.overload<GroupParamWithString>()
        .text("group")
        .required("add_del")
        .required("pluginName")
        .required("groupName")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, GroupParamWithString const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            if (origin.getOriginType() == CommandOriginType::Player) {
                auto& player = *static_cast<Player*>(origin.getEntity()); // entity* => Player&
                if (!player.isOperator()) return noPermission(output);
            }
            PermissionManager& manager = PermissionManager::getInstance();
            if (manager.hasRegisterPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);
                switch (param.add_del) {
                case OperationAddOrDel::add: {
                    bool status = core.createGroup(param.groupName);
                    if (status) {
                        output.success(
                            "Add group '{}' in plugin '{}' state '{}'",
                            param.groupName,
                            param.pluginName,
                            status ? "Success" : "Fail"
                        );
                    } else {
                        output.error(
                            "Add group '{}' in plugin '{}' state '{}'",
                            param.groupName,
                            param.pluginName,
                            status ? "Success" : "Fail"
                        );
                    }
                } break;
                case OperationAddOrDel::del: {
                    bool status = core.deleteGroup(param.groupName);
                    if (status) {
                        output.success(
                            "Delete group '{}' in plugin '{}' state '{}'",
                            param.groupName,
                            param.pluginName,
                            status ? "Success" : "Fail"
                        );
                    } else {
                        output.error(
                            "Delete group '{}' in plugin '{}' state '{}'",
                            param.groupName,
                            param.pluginName,
                            status ? "Success" : "Fail"
                        );
                    }
                } break;
                }
            } else {
                output.error("The plugin '{}' is not registered", param.pluginName.c_str());
            }
        }>();

    // permc list perm
    cmd.overload().text("list").text("perm").execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(
            output,
            origin.getOriginType(),
            CommandOriginType::Player,
            CommandOriginType::DedicatedServer
        );
        if (origin.getOriginType() == CommandOriginType::Player) {
            auto& player = *static_cast<Player*>(origin.getEntity()); // entity* => Player&
            if (!player.isOperator()) return noPermission(output);
        }
        PermissionRegister& registerInst = PermissionRegister::getInstance();
        auto                keys         = registerInst.getAllKeys();
        if (keys.empty()) return output.success("No registered plugins.");
        for (auto& key : keys) {
            output.success("Plugin: {}", key.c_str());
        }
        output.success("Total {} plugins registered.", keys.size());
    }>();

    // permc list plugin
    cmd.overload().text("list").text("plugin").execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(
            output,
            origin.getOriginType(),
            CommandOriginType::Player,
            CommandOriginType::DedicatedServer
        );
        if (origin.getOriginType() == CommandOriginType::Player) {
            auto& player = *static_cast<Player*>(origin.getEntity()); // entity* => Player&
            if (!player.isOperator()) return noPermission(output);
        }
        PermissionManager& manager = PermissionManager::getInstance();
        auto               keys    = manager.getAllKeys();
        if (keys.empty()) return output.success("No registered plugins.");
        for (auto& key : keys) {
            output.success("Plugin: {}", key.c_str());
        }
        output.success("Total {} plugins registered.", keys.size());
    }>();

    // permc list group <string pluginName> [string groupname]
    cmd.overload<ListGroups>()
        .text("list")
        .text("group")
        .required("pluginName")
        .optional("groupName")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, ListGroups const& param) {
            CHECK_COMMAND_TYPE(
                output,
                origin.getOriginType(),
                CommandOriginType::Player,
                CommandOriginType::DedicatedServer
            );
            if (origin.getOriginType() == CommandOriginType::Player) {
                auto& player = *static_cast<Player*>(origin.getEntity()); // entity* => Player&
                if (!player.isOperator()) return noPermission(output);
            }
            PermissionManager& manager = PermissionManager::getInstance();
            if (manager.hasRegisterPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);
                if (param.groupName.empty()) {
                    auto allGroups = core.getAllGroups();
                    if (allGroups.empty()) return output.success("No registered groups.");
                    for (auto& group : allGroups) {
                        output.success("Group: {}", group.groupName);
                    }
                    output.success("Total {} groups registered.", allGroups.size());
                } else if (core.hasGroup(param.groupName)) {
                    auto group = core.getGroup(param.groupName);
                    output.success("Group: {}", group->toString(2));
                }
            } else {
                output.error("The plugin '{}' is not registered", param.pluginName.c_str());
            }
        }>();
}
} // namespace perm::command