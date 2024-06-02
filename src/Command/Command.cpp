#include "Command.h"
#include "mc/server/commands/CommandPermissionLevel.h"

namespace pmc::command {

using string = std::string;
using ll::i18n_literals ::operator""_tr;
using PermissionCore     = pmc::PermissionCore;
using PermissionManager  = pmc::PermissionManager;
using PermissionRegister = pmc::PermissionRegister;

void noPermission(CommandOutput& output) { output.error("此命令仅限 \"OP\" 使用。"_tr()); }

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

enum CreatOrDelete : int { Create = 1, Delete = 0 };
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
    auto& cmd = ll::command::CommandRegistrar::getInstance()
                    .getOrCreateCommand("pmc", "PermissionCore", CommandPermissionLevel::Admin);
    auto core = pmc::PermissionManager::getInstance().getPermissionCore("PermissionCore");

    // pmc 打开Gui
    cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        Actor* entity = origin.getEntity();
        if (entity) {
            auto& player = *static_cast<Player*>(entity); // entity* => Player&
            if (player.isOperator()) pmc::form::index(player);
            else noPermission(output);
        }
    });

    // pmc list perm [pluginName] [permValue] 列出插件所有权限值、列出插件的权限详细信息
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

            // 列出所有插件
            if (param.pluginName.empty()) {
                auto   all = registerInst.getAllPluginNames();
                string plugins;
                plugins =
                    accumulate(all.begin(), all.end(), string(""), [](string a, string b) { return a + b + " "; });
                output.success("当前共有以下插件注册了权限: {0}"_tr(plugins));

                // 列出指定插件的权限
            } else if (!param.pluginName.empty() && param.permValue == -114514) {
                auto all = registerInst.getPermissions(param.pluginName);
                for (auto& perm : all) {
                    output.success(
                        "权限名: {0} | 权限值: {1} | 所属插件: {2}"_tr(perm.name, perm.value, param.pluginName)
                    );
                }
                output.success("共计 '{0}' 个权限。"_tr(all.size()));

                // 列出指定插件的指定权限
            } else {
                auto perm = registerInst.getPermission(param.pluginName, param.permValue);
                if (perm.has_value()) {
                    output.success(
                        "权限名: {0} | 权限值: {1} | 所属插件: {2}"_tr(perm->name, perm->value, param.pluginName)
                    );
                } else {
                    output.error("权限值: {0} 不存在。"_tr(param.permValue));
                }
            }
        });

    // pmc list group <pluginName> [groupName] 列出插件所有的组、列出插件的组详细信息
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
            if (manager.hasPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);
                if (param.groupName.empty()) {
                    auto allGroups = core.getAllGroups();
                    if (allGroups.empty()) return output.error("当前插件没有任何组。"_tr());
                    // vector to string example: "a b c"
                    string groups =
                        accumulate(allGroups.begin(), allGroups.end(), string(""), [](string a, pmc::group::Group& b) {
                            return a + b.groupName + " ";
                        });
                    output.success("当前插件共有以下组: {0}"_tr(groups));
                    output.success("共计 '{0}' 个组。"_tr(allGroups.size()));
                } else if (core.hasGroup(param.groupName)) {
                    auto group = core.getGroup(param.groupName);
                    output.success("组 {0} 的详细信息: {1}"_tr(group->groupName, group->toString(2)));
                } else {
                    output.error("组 {0} 不存在。"_tr(param.groupName));
                }
            } else {
                output.error("插件 {0} 不存在。"_tr(param.pluginName.c_str()));
            }
        });

    // pmc list plugin 列出所有插件
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
        auto               keys    = manager.getAllPluginNames();
        if (keys.empty()) return output.success("当前没有任何插件注册权限核心。"_tr());
        string plugins;
        plugins = accumulate(keys.begin(), keys.end(), string(""), [](string a, string b) { return a + b + " "; });
        output.success("当前共有以下插件注册了权限核心: {0}"_tr(plugins));
        output.success("共计 '{0}' 个插件。", keys.size());
    });

    // pmc group <create|delete> <pluginName> <groupName> 添加、删除组
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
            if (manager.hasPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);
                switch (param.c_or_d) {
                case CreatOrDelete::Create: {
                    bool status = core.createGroup(param.groupName);
                    if (status) output.success("成功为插件 '{0}' 创建组 '{1}'。"_tr(param.groupName, param.pluginName));
                    else output.error("为插件 '{0}' 创建组 '{1}' 失败。"_tr(param.groupName, param.pluginName));
                } break;
                case CreatOrDelete::Delete: {
                    bool status = core.deleteGroup(param.groupName);
                    if (status) output.success("成功从插件 '{0}' 删除组 '{1}'"_tr(param.groupName, param.pluginName));
                    else output.error("无法从插件 '{0}' 删除组 '{1}'"_tr(param.groupName, param.pluginName));
                } break;
                }
            } else {
                output.error("插件 {0} 不存在。"_tr(param.pluginName.c_str()));
            }
        });

    // pmc user <add|del> <pluginName> <groupName> <target Player> 添加、删除用户到组
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
            if (manager.hasPermissionCore(param.pluginName)) {
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
                                if (status)
                                    output.success("成功将用户 '{0}' 添加到插件 '{1}' 的组 '{2}' 中。"_tr(
                                        pl->getRealName(),
                                        param.pluginName,
                                        param.groupName
                                    ));
                                else
                                    output.error("无法将用户 '{0}' 添加到插件 '{1}' 的组 '{2}' 中。"_tr(
                                        pl->getRealName(),
                                        param.pluginName,
                                        param.groupName
                                    ));
                            } break;
                            case OperationAddOrDel::del: {
                                bool status = core.removeUserToGroup(param.groupName, uuid);
                                if (status)
                                    output.success("成功将用户 '{0}' 从插件 '{1}' 的组 '{2}' 中移除。"_tr(
                                        pl->getRealName(),
                                        param.pluginName,
                                        param.groupName
                                    ));
                                else
                                    output.error("无法将用户 '{0}' 从插件 '{1}' 的组 '{2}' 中移除。"_tr(
                                        pl->getRealName(),
                                        param.pluginName,
                                        param.groupName
                                    ));
                            } break;
                            }
                        }
                    }
                } else {
                    output.error("组 {0} 不存在。"_tr(param.groupName));
                }
            } else {
                output.error("插件 {0} 不存在。"_tr(param.pluginName));
            }
        });

    // pmc user <add|del> <pluginName> <groupName> <string realName> 添加、删除用户到组
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
            if (manager.hasPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);
                if (core.hasGroup(param.groupName)) {
                    auto pl = ll::service::getLevel()->getPlayer(param.realName);
                    if (pl) {
                        switch (param.add_del) {
                        case OperationAddOrDel::add: {
                            bool status =
                                core.addUserToGroup(param.groupName, param.realName, pl->getUuid().asString().c_str());
                            if (status)
                                output.success("成功将用户 '{0}' 添加到插件 '{1}' 的组 '{2}' 中。"_tr(
                                    pl->getRealName(),
                                    param.pluginName,
                                    param.groupName
                                ));
                            else
                                output.error("无法将用户 '{0}' 添加到插件 '{1}' 的组 '{2}' 中。"_tr(
                                    pl->getRealName(),
                                    param.pluginName,
                                    param.groupName
                                ));
                        } break;
                        case OperationAddOrDel::del: {
                            bool status = core.removeUserToGroup(param.groupName, pl->getUuid().asString().c_str());
                            if (status)
                                output.success("成功将用户 '{0}' 从插件 '{1}' 的组 '{2}' 中移除。"_tr(
                                    pl->getRealName(),
                                    param.pluginName,
                                    param.groupName
                                ));
                            else
                                output.error("无法将用户 '{0}' 从插件 '{1}' 的组 '{2}' 中移除。"_tr(
                                    pl->getRealName(),
                                    param.pluginName,
                                    param.groupName
                                ));
                        } break;
                        }
                    } else {
                        output.error("玩家 {0} 不存在。"_tr(param.realName));
                    }
                } else {
                    output.error("组 {0} 不存在。"_tr(param.groupName));
                }
            } else {
                output.error("插件 {0} 不存在。"_tr(param.pluginName));
            }
        });

    // pmc perm <add|del> <pluginName> <groupName> <value> 添加、删除权限到组
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
            if (manager.hasPermissionCore(param.pluginName)) {
                PermissionCore& core = *manager.getPermissionCore(param.pluginName);
                if (core.hasGroup(param.groupName)) {
                    PermissionRegister registerInst = PermissionRegister::getInstance();
                    if (registerInst.hasPermission(param.pluginName, param.permValue)) {
                        auto perm = registerInst.getPermission(param.pluginName, param.permValue);
                        switch (param.add_del) {
                        case OperationAddOrDel::add: {
                            bool status = core.addPermissionToGroup(param.groupName, perm->name, perm->value);
                            if (status)
                                output.success("添加权限 '{0}' 到插件 '{1}' 的组 '{2}' 成功。"_tr(
                                    perm->name,
                                    param.pluginName,
                                    param.groupName
                                ));
                            else
                                output.error("添加权限 '{0}' 到插件 '{1}' 的组 '{2}' 失败。"_tr(
                                    perm->name,
                                    param.pluginName,
                                    param.groupName
                                ));
                        } break;
                        case OperationAddOrDel::del: {
                            bool status = core.removePermissionToGroup(param.groupName, perm->value);
                            if (status)
                                output.success("删除权限 '{0}' 到插件 '{1}' 的组 '{2}' 成功。"_tr(
                                    perm->name,
                                    param.pluginName,
                                    param.groupName
                                ));
                            else
                                output.error("删除权限 '{0}' 到插件 '{1}' 的组 '{2}' 失败。"_tr(
                                    perm->name,
                                    param.pluginName,
                                    param.groupName
                                ));
                        }
                        }
                    } else {
                        output.error("权限值 {0} 不存在。"_tr(param.permValue));
                    }
                } else {
                    output.error("组 {0} 不存在。"_tr(param.groupName));
                }
            } else {
                output.error("插件 {0} 不存在。"_tr(param.pluginName));
            }
        });
}
} // namespace pmc::command