#pragma once

#include <cstdint>
#include <array>

namespace SteamIntegration {

enum class ActionSetId : std::uint8_t {
    Gameplay = 0,
    Count
};

enum class DigitalActionId : std::uint8_t {
    BodyRaise = 0,
    BodyLower,
    Count
};

enum class AnalogActionId : std::uint8_t {
    Move = 0,
    Camera,
    Count
};

struct ActionHandles {
    std::array<std::uint64_t, static_cast<std::size_t>(ActionSetId::Count)> actionSets{};
    std::array<std::uint64_t, static_cast<std::size_t>(DigitalActionId::Count)> digital{};
    std::array<std::uint64_t, static_cast<std::size_t>(AnalogActionId::Count)> analog{};
};

bool initialize();
void shutdown();
void pump();
bool isActive();

const ActionHandles& handles();
const char* actionSetName(ActionSetId id);
const char* digitalActionName(DigitalActionId id);
const char* analogActionName(AnalogActionId id);

}
