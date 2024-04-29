#include "Command.h"

namespace perm::command {

using string = std::string;
using ll::i18n_literals ::operator""_tr;
using PermissionCore     = perm::PermissionCore;
using PermissionManager  = perm::PermissionManager;
using PermissionRegister = perm::PermissionRegister;

void noPermission(CommandOutput& output) { output.error("This command is available to [OP] only!"_tr()); }

enum OperationAddOrDel : int { add = 0, del = 1 };
enum OperationUserOrPerm : int { user = 0, perm = 1 };

struct AddOrDelUserWithTarget {
    OperationAddOrDel       add_del;
    string                  pluginName;
    string                  groupName;
    CommandSelector<Player> player;
};
struct AddOrDelUserWithRealName {
    OperationAddOrDel add_del;
    string            pluginName;
    string            groupName;
    string            realName;
};

struct AddOrDelPermToGroup {
    OperationAddOrDel add_del;
    string            pluginName;
    string            groupName;
    int               permValue;
};

enum CreatOrDelete : int { Creat = 1, Delete = 0 };
struct CreatOrDeleteGroup {
    CreatOrDelete c_or_d;
    string        pluginName;
    string        groupName;
};

struct ListGroup {
    string pluginName;
    string groupName;
};

struct ListPerm {
    string pluginName;
    int    permValue = -114514;
};

struct ListPlugin {
    string pluginName;
};

void registerCommand() {
    auto& cmd  = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("permc");
    auto  core = perm::PermissionManager::getInstance().getPermissionCore("PermissionCore");

    // permc 打开Gui
    cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        Actor* entity = origin.getEntity();
        if (entity) {
            auto& player = *static_cast<Player*>(entity); // entity* => Player&
            if (player.isOperator()) perm::form::index(player);
            else noPermission(output);
        }
    });

    // permc list perm [pluginName] [permValue] 列出插件所有权限值、列出插件的权限详细信息
    cmd.overload<ListPerm>()
        .text("list")
        .text("perm")
        .optional("pluginName")
        .optional("permValue")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, const ListPerm& param) {
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
            if (param.pluginName.empty()) {
                auto   all = registerInst.getAllKeys();
                string plugins;
                plugins =
                    accumulate(all.begin(), all.end(), string(""), [](string a, string b) { return a + b + " "; });
                output.success(
                    "There are currently a total of the following plugins registered with permissions: {}"_tr(plugins)
                );
            } else if (param.permValue == -114514) {
                auto all = registerInst.getAllPermission(param.pluginName);
                for (auto& perm : all) {
                    output.success("PermissionName: {} | PermissionValue: {} | With Plugin: {}"_tr(
                        perm.name,
                        perm.value,
                        param.pluginName
                    ));
                }
                output.success("Total {} permissions registered."_tr(all.size()));
            } else {
                auto perm = registerInst.getPermission(param.pluginName, param.permValue);
                if (perm.has_value()) {
                    output.success("PermissionName: {} | PermissionValue: {} | With Plugin: {}"_tr(
                        perm->name,
                        perm->value,
                        param.pluginName
                    ));
                } else {
                    output.error("The permission value '{}' is not registered"_tr(param.permValue));
                }
            }
        });

    // permc list group <pluginName> [groupName] 列出插件所有的组、列出插件的组详细信息
    cmd.overload<ListGroup>()
        .text("list")
        .text("group")
        .required("pluginName")
        .optional("groupName")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, ListGroup const& param) {
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
                    if (allGroups.empty()) return output.success("No registered groups."_tr());
                    // vector to string example: "a b c"
                    string groups =
                        accumulate(allGroups.begin(), allGroups.end(), string(""), [](string a, perm::group::Group& b) {
                            return a + b.groupName + " ";
                        });
                    output.success("Group: {}"_tr(groups));
                    output.success("Total {} groups registered."_tr(allGroups.size()));
                } else if (core.hasGroup(param.groupName)) {
                    auto group = core.getGroup(param.groupName);
                    output.success("Group: {}"_tr(group->toString(2)));
                } else {
                    output.error("The group '{}' is not registered"_tr(param.groupName));
                }
            } else {
                output.error("The plugin '{}' is not registered"_tr(param.pluginName.c_str()));
            }
        });

    // permc list plugin 列出所有插件
    cmd.overload<ListPlugin>().text("list").text("plugin").execute([&](CommandOrigin const& origin,
                                                                       CommandOutput&       output) {
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
        if (keys.empty()) return output.success("No registered plugins."_tr());
        string plugins;
        plugins = accumulate(keys.begin(), keys.end(), string(""), [](string a, string b) { return a + b + " "; });
        output.success("A total of the following plugins are currently registered with Permission Core: {}"_tr(plugins)
        );
        output.success("Total {} plugins registered.", keys.size());
    });

    // permc group <create|delete> <pluginName> <groupName> 添加、删除组
    cmd.overload<CreatOrDeleteGroup>()
        .text("group")
        .required("c_or_d")
        .required("pluginName")
        .required("groupName")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, CreatOrDeleteGroup const& param) {
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
                switch (param.c_or_d) {
                case CreatOrDelete::Creat: {
                    bool status = core.createGroup(param.groupName);
                    output.success("Add group '{}' in plugin '{}' state '{}'"_tr(
                        param.groupName,
                        param.pluginName,
                        status ? "Success"_tr() : "Fail"_tr()
                    ));
                } break;
                case CreatOrDelete::Delete: {
                    bool status = core.deleteGroup(param.groupName);
                    output.success("Delete group '{}' in plugin '{}' state '{}'"_tr(
                        param.groupName,
                        param.pluginName,
                        status ? "Success"_tr() : "Fail"_tr()
                    ));
                } break;
                }
            } else {
                output.error("The plugin '{}' is not registered"_tr(param.pluginName.c_str()));
            }
        });

    // permc user <add|del> <pluginName> <groupName> <target Player> 添加、删除用户到组
    cmd.overload<AddOrDelUserWithTarget>()
        .text("user")
        .required("add_del")
        .required("pluginName")
        .required("groupName")
        .required("player")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, AddOrDelUserWithTarget const& param) {
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
                                output.success("Add user '{}' to group '{}' in plugin '{}' state '{}'"_tr(
                                    pl->getRealName(),
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success"_tr() : "Fail"_tr()
                                ));
                            } break;
                            case OperationAddOrDel::del: {
                                bool status = core.removeUserToGroup(param.groupName, uuid);
                                output.success("Remove user '{}' to group '{}' in plugin '{}' state '{}'"_tr(
                                    pl->getRealName(),
                                    param.groupName,
                                    param.pluginName,
                                    status ? "Success"_tr() : "Fail"_tr()
                                ));
                            } break;
                            }
                        }
                    }
                } else {
                    output.error("The group '{}' is not registered"_tr(param.groupName));
                }
            } else {
                output.error("The target plugin '{}' is not registered"_tr(param.pluginName));
            }
        });

    // permc user <add|del> <pluginName> <groupName> <string realName> 添加、删除用户到组
    cmd.overload<AddOrDelUserWithRealName>()
        .text("user")
        .required("add_del")
        .required("pluginName")
        .required("groupName")
        .required("realName")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, AddOrDelUserWithRealName const& param) {
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
                            output.success("Add user '{}' to group '{}' in plugin '{}' state '{}'"_tr(
                                param.realName,
                                param.groupName,
                                param.pluginName,
                                status ? "Success"_tr() : "Fail"_tr()
                            ));
                        } break;
                        case OperationAddOrDel::del: {
                            bool status = core.removeUserToGroup(param.groupName, pl->getUuid().asString().c_str());
                            output.success("Remove user '{}' to group '{}' in plugin '{}' state '{}'"_tr(
                                param.realName,
                                param.groupName,
                                param.pluginName,
                                status ? "Success"_tr() : "Fail"_tr()
                            ));
                        } break;
                        }
                    } else {
                        output.error("The player '{}' is not found"_tr(param.realName));
                    }
                } else {
                    output.error("The group '{}' is not registered"_tr(param.groupName));
                }
            } else {
                output.error("The target plugin '{}' is not registered"_tr(param.pluginName));
            }
        });

    // permc perm <add|del> <pluginName> <groupName> <value> 添加、删除权限到组
    cmd.overload<AddOrDelPermToGroup>()
        .text("perm")
        .required("add_del")
        .required("pluginName")
        .required("groupName")
        .required("permValue")
        .execute([&](CommandOrigin const& origin, CommandOutput& output, AddOrDelPermToGroup const& param) {
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
                            output.success("Add permission '{}' to group '{}' in plugin '{}' state '{}'"_tr(
                                perm->name,
                                param.groupName,
                                param.pluginName,
                                status ? "Success"_tr() : "Fail"_tr()
                            ));
                        } break;
                        case OperationAddOrDel::del: {
                            bool status = core.removePermissionToGroup(param.groupName, perm->value);
                            output.success("Remove permission '{}' to group '{}' in plugin '{}' state '{}'"_tr(
                                perm->name,
                                param.groupName,
                                param.pluginName,
                                status ? "Success"_tr() : "Fail"_tr()
                            ));
                        }
                        }
                    } else {
                        output.error("The permission '{}' is not registered"_tr(param.permValue));
                    }
                } else {
                    output.error("The group '{}' is not registered"_tr(param.groupName));
                }
            } else {
                output.error("The plugin '{}' is not registered"_tr(param.pluginName));
            }
        });
}
} // namespace perm::command