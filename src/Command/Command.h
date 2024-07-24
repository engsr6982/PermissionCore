#pragma once
#include "Form/Global.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/PermissionRegister.h"
#include "ll/api/i18n/I18n.h"
#include <initializer_list>
#include <ll/api/Logger.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/command/Optional.h>
#include <ll/api/command/Overload.h>
#include <ll/api/i18n/I18n.h>
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


namespace pmc::command {

using string = std::string;
using ll::i18n_literals ::operator""_tr;

void registerCommand();

// #define C_RESET  "\033[0m"
// #define C_BLACK  "\033[30m" /* Black */
// #define C_RED    "\033[31m" /* Red */
// #define C_GREEN  "\033[32m" /* Green */
// #define C_YELLOW "\033[33m" /* Yellow */
// #define C_BLUE   "\033[34m" /* Blue */
// #define C_PURPLE "\033[35m" /* Purple */
// #define C_CYAN   "\033[36m" /* Cyan */
// #define C_WHITE  "\033[37m" /* White */

inline string CommandOriginTypeToString(CommandOriginType type) {
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
            __output.error("This command is available to '{}' only!"_tr(__allowedTypesStr.str()));                     \
            return;                                                                                                    \
        }                                                                                                              \
    }

} // namespace pmc::command