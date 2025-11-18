#include "steamIntegration.h"

#include <cstdio>
#include <fstream>

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

bool SteamIntegration::initialize(const std::string& manifestPath) {
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

  _manifestPath = manifestPath;
  if (SteamInput()) {
    std::ifstream manifestStream(_manifestPath);
    if(manifestStream.good()) {
      if(SteamInput()->SetInputActionManifestFilePath(_manifestPath.c_str()))
        std::printf("SteamIntegration: action manifest %s registered\n", _manifestPath.c_str());
      else
        std::printf("SteamIntegration: failed to register action manifest %s\n", _manifestPath.c_str());
    } else {
      std::printf("SteamIntegration: action manifest missing at %s\n", _manifestPath.c_str());
    }
    _steamInputAvailable = SteamInput()->Init(false);
  } else {
    _steamInputAvailable = false;
  }
  if (!_steamInputAvailable) {
    std::printf("SteamIntegration: SteamInput unavailable, continuing without it\n");
  }

  if (_steamInputAvailable) {
    _actionSetHandle = SteamInput()->GetActionSetHandle("ship_controls");
    _moveAnalogHandle = SteamInput()->GetAnalogActionHandle("analog_controls");
    _lookAnalogHandle = 0;

    if(!_actionSetHandle)
      std::printf("SteamIntegration: action set handle for ship_controls unavailable\n");
    if(!_moveAnalogHandle)
      std::printf("SteamIntegration: analog handle for analog_controls unavailable\n");
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

  static bool missingAnalogLog = false;
  if(!_actionSetHandle || !_moveAnalogHandle) {
    if(!missingAnalogLog) {
      std::printf("SteamIntegration: analog action handles not ready; controller data unavailable\n");
      missingAnalogLog = true;
    }
    return;
  }

  for(int i = 0; i < count; ++i) {
    const InputHandle_t handle = handles[i];
    SteamInput()->ActivateActionSet(handle, _actionSetHandle);

    const InputAnalogActionData_t moveData = SteamInput()->GetAnalogActionData(handle, _moveAnalogHandle);
    if(moveData.bActive) {
      _inputState.hasAnalog = true;
      _inputState.leftX = moveData.x;
      _inputState.leftY = moveData.y;
      _inputState.rightX = moveData.x;
      _inputState.rightY = moveData.y;
    }

    if(_inputState.hasAnalog) break;
  }
#else
  resetInputState();
#endif
}
