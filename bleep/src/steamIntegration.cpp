#include "steamIntegration.h"

#include <cstdio>

#ifdef BLEEP_WITH_STEAM
#include <steam/steam_api.h>
#endif

namespace {
constexpr unsigned int kBleepSteamAppID = 480;
}

SteamIntegration::SteamIntegration():
  _runtimeActive(false),
  _steamInputAvailable(false)
{
  resetInputState();
}

SteamIntegration::~SteamIntegration() {
  shutdown();
}

bool SteamIntegration::initialize() {
#ifdef BLEEP_WITH_STEAM
  if (_runtimeActive) {
    return true;
  }

  if (SteamAPI_RestartAppIfNecessary(kBleepSteamAppID)) {
    _runtimeActive = false;
    return false;
  }

  if (!SteamAPI_Init()) {
    std::printf("SteamIntegration: SteamAPI_Init failed\n");
    _runtimeActive = false;
    return false;
  }

  _steamInputAvailable = SteamInput() && SteamInput()->Init(false);
  if (!_steamInputAvailable) {
    std::printf("SteamIntegration: SteamInput unavailable, continuing without it\n");
  }

  _runtimeActive = true;
  return true;
#else
  _runtimeActive = false;
  return false;
#endif
}

void SteamIntegration::shutdown() {
#ifdef BLEEP_WITH_STEAM
  if (_steamInputAvailable && SteamInput()) {
    SteamInput()->Shutdown();
  }
  if (_runtimeActive) {
    SteamAPI_Shutdown();
  }
#endif
  _steamInputAvailable = false;
  _runtimeActive = false;
  resetInputState();
}

void SteamIntegration::update() {
#ifdef BLEEP_WITH_STEAM
  if (!_runtimeActive) {
    return;
  }

  SteamAPI_RunCallbacks();
  if (_steamInputAvailable && SteamInput()) {
    SteamInput()->RunFrame();
  }
#endif
}

bool SteamIntegration::isRunning() const {
  return _runtimeActive;
}

const SteamInputState& SteamIntegration::getInputState() const {
  return _inputState;
}

void SteamIntegration::resetInputState() {
  _inputState = SteamInputState{};
}
