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
#ifdef BLEEP_WITH_STEAM
  _actionSetHandle = 0;
  _moveAnalogHandle = 0;
  _lookAnalogHandle = 0;
#endif
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

  if (SteamInput()) {
#ifdef STEAM_ACTION_MANIFEST
    SteamInput()->SetInputActionManifestFilePath(STEAM_ACTION_MANIFEST);
#endif
    _steamInputAvailable = SteamInput()->Init(false);
  } else {
    _steamInputAvailable = false;
  }
  if (!_steamInputAvailable) {
    std::printf("SteamIntegration: SteamInput unavailable, continuing without it\n");
  }

  if (_steamInputAvailable) {
    _actionSetHandle = SteamInput()->GetActionSetHandle("Gameplay");
    _moveAnalogHandle = SteamInput()->GetAnalogActionHandle("Move");
    _lookAnalogHandle = SteamInput()->GetAnalogActionHandle("Look");
    if(!_actionSetHandle || !_moveAnalogHandle || !_lookAnalogHandle) {
      std::printf("SteamIntegration: analog handles unavailable; Steam Input will fall back to SDL\n");
    }
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

  pollControllers();
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

void SteamIntegration::pollControllers() {
#ifdef BLEEP_WITH_STEAM
  if(!_steamInputAvailable || !SteamInput()) {
    resetInputState();
    return;
  }

  InputHandle_t handles[STEAM_INPUT_MAX_COUNT]{};
  const int count = SteamInput()->GetConnectedControllers(handles);
  resetInputState();
  if(count <= 0) {
    return;
  }

  _inputState.connected = true;

  if(!_actionSetHandle || !_moveAnalogHandle || !_lookAnalogHandle) {
    return;
  }

  for(int i = 0; i < count; ++i) {
    const InputHandle_t handle = handles[i];
    SteamInput()->ActivateActionSet(handle, _actionSetHandle);

    const InputAnalogActionData_t moveData = SteamInput()->GetAnalogActionData(handle, _moveAnalogHandle);
    const InputAnalogActionData_t lookData = SteamInput()->GetAnalogActionData(handle, _lookAnalogHandle);

    if(moveData.bActive) {
      _inputState.hasAnalog = true;
      _inputState.leftX = moveData.x;
      _inputState.leftY = moveData.y;
    }
    if(lookData.bActive) {
      _inputState.hasAnalog = true;
      _inputState.rightX = lookData.x;
      _inputState.rightY = lookData.y;
    }

    if(_inputState.hasAnalog) break;
  }
#else
  resetInputState();
#endif
}
