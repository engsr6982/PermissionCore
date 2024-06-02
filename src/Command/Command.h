#pragma once
#include "Form/index.h"
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


namespace pmc::command {

using string = std::string;
using ll::i18n_literals ::operator""_tr;

void registerCommand();

inline string CommandOriginTypeToString(CommandOriginType type) {
    switch (type) {
    case CommandOriginType::Player:
        return "Player";
    case CommandOriginType::CommandBlock:
        return "CommandBlock";
    case CommandOriginType::MinecartCommandBlock:
        return "MinecartCommandBlock";
    case CommandOriginType::DevConsole:
        return "DevConsole";
    case CommandOriginType::Test:
        return "Test";
    case CommandOriginType::AutomationPlayer:
        return "AutomationPlayer";
    case CommandOriginType::ClientAutomation:
        return "ClientAutomation";
    case CommandOriginType::DedicatedServer:
        return "DedicatedServer";
    case CommandOriginType::Entity:
        return "Entity";
    case CommandOriginType::Virtual:
        return "Virtual";
    case CommandOriginType::GameArgument:
        return "GameArgument";
    case CommandOriginType::EntityServer:
        return "EntityServer";
    case CommandOriginType::Precompiled:
        return "Precompiled";
    case CommandOriginType::GameDirectorEntityServer:
        return "GameDirectorEntityServer";
    case CommandOriginType::Scripting:
        return "Scripting";
    case CommandOriginType::ExecuteContext:
        return "ExecuteContext";
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
            __output.error("此命令仅限 '{0}' 使用!"_tr(__allowedTypesStr.str()));                                      \
            return;                                                                                                    \
        }                                                                                                              \
    }

} // namespace pmc::command