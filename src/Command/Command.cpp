#include "Command.h"
#include "Form/Global.h"
#include "PermissionCore/PermissionManager.h"
#include "entry/Permission.h"
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

#define CHECK_COMMAND_TYPE(output, originType, ...)                                                                    \
    {                                                                                                                  \
        std::initializer_list<CommandOriginType> allowedTypes = {__VA_ARGS__};                                         \
        bool                                     typeMatched  = false;                                                 \
        for (auto allowedType : allowedTypes) {                                                                        \
            if (originType == allowedType) {                                                                           \
                typeMatched = true;                                                                                    \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        if (!typeMatched) {                                                                                            \
            std::stringstream allowedTypesStr;                                                                         \
            bool              first = true;                                                                            \
            for (auto allowedType : allowedTypes) {                                                                    \
                if (!first) allowedTypesStr << ", ";                                                                   \
                allowedTypesStr << CommandOriginTypeToString(allowedType);                                             \
                first = false;                                                                                         \
            }                                                                                                          \
            output.error("This command is available to [" + allowedTypesStr.str() + "] only!");                        \
            return;                                                                                                    \
        }                                                                                                              \
    }

void noPermission(CommandOutput& output) {
    output.error("No permission, this command requires permission 'commad'.");
    output.error(
        "Use the command 'permc admin add permissioncore your_game_name command' to add permissions and try again"
    );
}

using string            = std::string;
using PermissionManager = perm::PermissionManager;
using PermissionCore    = perm::PermissionCore;

enum OperationType : int { add = 0, del = 1 };

struct AdminWithTarget {
    OperationType           operation;
    string                  pluginName;
    CommandSelector<Player> player;
};
struct AdminWithString {
    OperationType operation;
    string        pluginName;
    string        uuid;
};

void registerCommand() {

    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand("permc");

    // permc <add|del> <user|perm> <string pluginName> <string groupName> <string uuid_or_perm>
    // cmd.overload<AdminWithTarget>()
    //     .text("admin")
    //     .required("operation")
    //     .required("pluginName")
    //     .required("player")
    //     .execute<[&](CommandOrigin const& origin, CommandOutput& output, AdminWithTarget const& param) {
    //         CHECK_COMMAND_TYPE(
    //             output,
    //             origin.getOriginType(),
    //             CommandOriginType::DedicatedServer,
    //             CommandOriginType::Player
    //         );
    //         // 检查执行者是否是操作员
    //         if (origin.getOriginType() == CommandOriginType::Player) {
    //             auto& player = *static_cast<Player*>(origin.getEntity());
    //             if (!player.isOperator()) return output.error("This command is available to [OP] only!");
    //         }
    //         PermissionManager& manager = PermissionManager::getInstance();
    //         if (manager.hasRegisterPermissionCore(param.pluginName)) {
    //             PermissionCore& core   = *manager.getPermissionCore(param.pluginName);
    //             auto            target = param.player.results(origin).data;

    //             for (Player* pl : *target) {
    //                 if (pl) {
    //                     string uuid = pl->getUuid().asString().c_str();
    //                     switch (param.operation) {
    //                     case OperationType::add: {
    //                         if (core.isAdmin(uuid)) {
    //                             output.error(
    //                                 "Player '{}' is already a plugin '{}' administrator",
    //                                 pl->getRealName(),
    //                                 param.pluginName
    //                             );
    //                         } else {
    //                             string status = core.addAdmin(uuid) ? "Success" : "Fail";
    //                             output.success(
    //                                 "Add admin '{}' to plugin '{}' state '{}'",
    //                                 pl->getRealName(),
    //                                 param.pluginName,
    //                                 status
    //                             );
    //                         }
    //                     } break;
    //                     case OperationType::del: {
    //                         if (core.isAdmin(uuid)) {
    //                             string status = core.removeAdmin(uuid) ? "Success" : "Fail";
    //                             output.success(
    //                                 "Remove admin '{}' from plugin '{}', state '{}'",
    //                                 pl->getRealName(),
    //                                 param.pluginName,
    //                                 status
    //                             );
    //                         } else {
    //                             output.error(
    //                                 "Player '{}' is not a plugin '{}' administrator",
    //                                 pl->getRealName(),
    //                                 param.pluginName
    //                             );
    //                         }
    //                     } break;
    //                     }
    //                 }
    //             }
    //         } else {
    //             output.error("The target plugin '{}' is not registered", param.pluginName);
    //         }
    //     }>();

    // permc gui [string pluginName]

    // permc
    // cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
    //     CHECK_COMMAND_TYPE(output, origin.getOriginType(), CommandOriginType::Player);
    //     Actor* entity = origin.getEntity();
    //     if (entity) {
    //         auto& player = *static_cast<Player*>(entity); // entity* => Player&
    //         auto& core   = *PermissionManager::getInstance().getPermissionCore("permissioncore");
    //         if (core.checkUserPermission(player.getUuid().asString().c_str(), "command", false, true)) {
    //             perm::form::index(player);
    //         } else {
    //             noPermission(output);
    //         }
    //     }
    // }>();
}

} // namespace perm::command