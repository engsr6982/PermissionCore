#include "Command.h"
#include "Form/Global.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/PermissionRegister.h"
#include <initializer_list>
#include <ll/api/Logger.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/command/Optional.h>
#include <ll/api/command/Overload.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/service/PlayerInfo.h>
#include <ll/api/service/Service.h>
#include <ll/api/utils/HashUtils.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/network/packet/LevelChunkPacket.h>
#include <mc/network/packet/TextPacket.h>
#include <mc/server/ServerLevel.h>
#include <mc/server/ServerPlayer.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOriginType.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandParameterOption.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/server/commands/CommandRegistry.h>
#include <mc/server/commands/CommandSelector.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>
#include <sstream>


namespace perm::command {

std::string CommandOriginTypeToString(CommandOriginType type) {
    switch (type) {
    case CommandOriginType::Player:
        return "players";
    case CommandOriginType::CommandBlock:
        return "command blocks";
    case CommandOriginType::MinecartCommandBlock:
        return "minecart command blocks";
    case CommandOriginType::DevConsole:
        return "the developer console";
    case CommandOriginType::Test:
        return "test origins";
    case CommandOriginType::AutomationPlayer:
        return "automation players";
    case CommandOriginType::ClientAutomation:
        return "client automation";
    case CommandOriginType::DedicatedServer:
        return "dedicated servers";
    case CommandOriginType::Entity:
        return "entities";
    case CommandOriginType::Virtual:
        return "virtual origins";
    case CommandOriginType::GameArgument:
        return "game argument origins";
    case CommandOriginType::EntityServer:
        return "entity servers";
    case CommandOriginType::Precompiled:
        return "precompiled origins";
    case CommandOriginType::GameDirectorEntityServer:
        return "game director entity servers";
    case CommandOriginType::Scripting:
        return "scripting origins";
    case CommandOriginType::ExecuteContext:
        return "execute contexts";
    default:
        return "unknown";
    }
}

#define CHECK_COMMAND_TYPE(__output, __originType, ...)                                                                \
    {                                                                                                                  \
        std::initializer_list<CommandOriginType> __allowedTypes = {__VA_ARGS__};                                       \
        bool                                     __typeMatched  = false;                                               \
        for (auto _allowedType : __allowedTypes) {                                                                     \
            if (__originType == _allowedType) {                                                                        \
                __typeMatched = true;                                                                                  \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        if (!__typeMatched) {                                                                                          \
            std::stringstream __allowedTypesStr;                                                                       \
            bool              __first = true;                                                                          \
            for (auto __allowedType : __allowedTypes) {                                                                \
                if (!__first) __allowedTypesStr << ", ";                                                               \
                __allowedTypesStr << CommandOriginTypeToString(__allowedType);                                         \
                __first = false;                                                                                       \
            }                                                                                                          \
            __output.error("This command is available to [" + __allowedTypesStr.str() + "] only!");                    \
            return;                                                                                                    \
        }                                                                                                              \
    }

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

enum TranslateType : int { realname = 0, uuid = 1 };
struct TranslateParam {
    TranslateType type;
    string        name_uuid;
};

void registerCommand() {
    auto& cmd  = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("permc");
    auto  core = perm::PermissionManager::getInstance().getPermissionCore("permissioncore");

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

    // permc list perms
    cmd.overload().text("list").text("perms").execute<[&](CommandOrigin const& origin, CommandOutput& output) {
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

    // permc list plugins
    cmd.overload().text("list").text("plugins").execute<[&](CommandOrigin const& origin, CommandOutput& output) {
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

    // permc list groups <string pluginName> [string groupname]
    cmd.overload<PermParamWithString>()
        .text("list")
        .text("groups")
        .required("pluginName")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, PermParamWithString const& param) {
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
                } else {
                    if (core.hasGroup(param.groupName)) {
                        auto group = core.getGroup(param.groupName);
                        output.success("Group: {}", group->toString(2));
                    }
                }
            } else {
                output.error("The plugin '{}' is not registered", param.pluginName.c_str());
            }
        }>();

    // permc translate <realname|uuid> <string name_uuid>
    cmd.overload<TranslateParam>()
        .text("translate")
        .required("type")
        .required("name_uuid")
        .execute<[&](CommandOrigin const& origin, CommandOutput& output, TranslateParam const& param) {
            try {
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
                if (param.name_uuid.empty()) return output.error("The name_uuid cannot be empty.");
                switch (param.type) {
                case TranslateType::realname: {
                    output.success(
                        "Translate realName '{}' to uuid '{}'",
                        param.name_uuid,
                        ll::service::PlayerInfo::getInstance().fromName(param.name_uuid)->uuid.asString().c_str()
                    );
                } break;
                case TranslateType::uuid: {
                    output.success(
                        "Translate uuid '{}' to realName '{}'",
                        param.name_uuid,
                        ll::service::PlayerInfo::getInstance().fromUuid(param.name_uuid)->name.c_str()
                    );
                }
                }
            } catch (std::exception const& e) {
                output.error("Faild to translate '{}', stack: {}", param.name_uuid.c_str(), e.what());
            } catch (...) {
                output.error("Faild to translate '{}', Unknown error", param.name_uuid.c_str());
            }
        }>();

    // permc
    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
        Actor* entity = origin.getEntity();
        if (entity) {
            auto& player = *static_cast<Player*>(entity); // entity* => Player&
            if (player.isOperator()) {
                perm::form::index(player);
            } else {
                noPermission(output);
            }
        }
    }>();
}
} // namespace perm::command