#ifndef steam_integration_h
#define steam_integration_h

class SteamIntegration {
public:
  SteamIntegration();
  ~SteamIntegration();

  void initialize();
  void shutdown();

  bool isRunning() const;

private:
  bool _runtimeActive;
};

#endif
