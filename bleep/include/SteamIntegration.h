#pragma once

#include <cstdint>
#include <array>

namespace SteamIntegration {

enum class ActionSetId : std::uint8_t {
    Gameplay = 0,
    Menu,
    Count
};

enum class DigitalActionId : std::uint16_t {
    ButtonSouth = 0,
    ButtonEast,
    ButtonWest,
    ButtonNorth,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
    LeftBumper,
    RightBumper,
    LeftTriggerClick,
    RightTriggerClick,
    LeftStickClick,
    RightStickClick,
    Menu,
    View,
    Steam,
    QuickAccess,
    LeftPaddleUpper,
    LeftPaddleLower,
    RightPaddleUpper,
    RightPaddleLower,
    LeftTrackpadClick,
    RightTrackpadClick,
    Count
};

enum class AnalogActionId : std::uint16_t {
    LeftStick = 0,
    RightStick,
    LeftTrigger,
    RightTrigger,
    LeftTrackpad,
    RightTrackpad,
    Gyro,
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
