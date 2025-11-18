#ifndef steam_integration_h
#define steam_integration_h

#ifdef BLEEP_WITH_STEAM
#include <steam/steam_api.h>
#endif

#include <string>

struct SteamInputState {
  bool connected = false;
  bool hasAnalog = false;
  float leftX = 0.0f;
  float leftY = 0.0f;
  float rightX = 0.0f;
  float rightY = 0.0f;
};

class SteamIntegration {
public:
  SteamIntegration();
  ~SteamIntegration();

  bool initialize(const std::string& manifestPath);
  void shutdown();
  void update();

  bool isRunning() const;
  const SteamInputState& getInputState() const;

private:
  void resetInputState();
  void pollControllers();

  bool _runtimeActive;
  bool _steamInputAvailable;
  std::string _manifestPath;
  SteamInputState _inputState;
#ifdef BLEEP_WITH_STEAM
  InputActionSetHandle_t _actionSetHandle;
  InputAnalogActionHandle_t _moveAnalogHandle;
  InputAnalogActionHandle_t _lookAnalogHandle;
#endif
};

#endif
