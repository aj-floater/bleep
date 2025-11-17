#ifndef steam_integration_h
#define steam_integration_h

struct SteamInputState {
  bool connected = false;
  float leftX = 0.0f;
  float leftY = 0.0f;
  float rightX = 0.0f;
  float rightY = 0.0f;
};

class SteamIntegration {
public:
  SteamIntegration();
  ~SteamIntegration();

  bool initialize();
  void shutdown();
  void update();

  bool isRunning() const;
  const SteamInputState& getInputState() const;

private:
  void resetInputState();

  bool _runtimeActive;
  bool _steamInputAvailable;
  SteamInputState _inputState;
};

#endif
