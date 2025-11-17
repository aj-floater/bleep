#include "steamIntegration.h"

SteamIntegration::SteamIntegration():
  _runtimeActive(false)
{}

SteamIntegration::~SteamIntegration() {
  shutdown();
}

void SteamIntegration::initialize() {
#ifdef BLEEP_WITH_STEAM
  // Placeholder: future Steam API init lives here.
  _runtimeActive = true;
#else
  _runtimeActive = false;
#endif
}

void SteamIntegration::shutdown() {
#ifdef BLEEP_WITH_STEAM
  // Placeholder for Steam API shutdown logic.
#endif
  _runtimeActive = false;
}

bool SteamIntegration::isRunning() const {
  return _runtimeActive;
}
