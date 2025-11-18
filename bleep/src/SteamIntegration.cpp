#include "SteamIntegration.h"

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include <Corrade/Utility/Debug.h>

#if defined(BLEEP_WITH_STEAM)
#include <steam/steam_api.h>
#endif

namespace SteamIntegration {
namespace {
    bool gSteamInitialized = false;
    bool gSteamInputInitialized = false;
    ActionHandles gHandles{};

    constexpr std::array<const char*, static_cast<std::size_t>(ActionSetId::Count)> ActionSetNames{
        "bleep_gameplay"
    };

    constexpr std::array<const char*, static_cast<std::size_t>(DigitalActionId::Count)> DigitalActionNames{
        "body_raise",
        "body_lower"
    };

    constexpr std::array<const char*, static_cast<std::size_t>(AnalogActionId::Count)> AnalogActionNames{
        "move",
        "camera"
    };

#if defined(BLEEP_WITH_STEAM)
    using Corrade::Utility::Debug;
    using Corrade::Utility::Warning;

    void logMissingInstall()
    {
        Warning{}
            << "Steam support enabled but Steam is not running or the SDK is misconfigured.";
    }

    std::string sourceDir()
    {
        std::string file = __FILE__;
        const auto lastSlash = file.find_last_of("/\\");
        if(lastSlash == std::string::npos) return {};
        return file.substr(0, lastSlash);
    }

    bool fileExists(const std::string& path)
    {
        if(path.empty()) return false;
        std::ifstream file{path};
        return file.good();
    }

    std::string findManifestPath()
    {
        std::vector<std::string> candidates;
#ifdef BLEEP_STEAM_INPUT_MANIFEST_PATH
        candidates.emplace_back(BLEEP_STEAM_INPUT_MANIFEST_PATH);
#endif
        candidates.emplace_back("steam_input_manifest.vdf");
        candidates.emplace_back("bleep/steam_input_manifest.vdf");

        std::string srcDir = sourceDir();
        if(!srcDir.empty())
            candidates.emplace_back(srcDir + "/../steam_input_manifest.vdf");

        for(const auto& candidate : candidates)
        {
            if(fileExists(candidate))
                return candidate;
        }

        return {};
    }

    bool configureManifest()
    {
        const std::string manifestPath = findManifestPath();
        if(manifestPath.empty())
        {
            Warning{} << "[SteamIntegration] Steam Input manifest not found. Actions will remain inactive.";
            return false;
        }

        SteamInput()->SetInputActionManifestFilePath(manifestPath.c_str());
        Debug{} << "[SteamIntegration] Steam Input manifest set to" << manifestPath.c_str();
        return true;
    }

    template<class Id, class Getter>
    void loadHandles(const std::array<const char*, static_cast<std::size_t>(Id::Count)>& names,
                     std::array<std::uint64_t, static_cast<std::size_t>(Id::Count)>& storage,
                     const char* label,
                     Getter getter)
    {
        for(std::size_t i = 0; i < names.size(); ++i)
        {
            const auto handle = getter(names[i]);
            storage[i] = handle;
            if(handle == 0)
            {
                Warning{} << "[SteamIntegration]" << label << "handle missing for" << names[i];
            }
            else
            {
                Debug{} << "[SteamIntegration]" << label << names[i] << "loaded as" << handle;
            }
        }
    }

    void loadActionHandles()
    {
        loadHandles<ActionSetId>(ActionSetNames, gHandles.actionSets, "Action set", [](const char* name) {
            return SteamInput()->GetActionSetHandle(name);
        });

        loadHandles<DigitalActionId>(DigitalActionNames, gHandles.digital, "Digital action", [](const char* name) {
            return SteamInput()->GetDigitalActionHandle(name);
        });

        loadHandles<AnalogActionId>(AnalogActionNames, gHandles.analog, "Analog action", [](const char* name) {
            return SteamInput()->GetAnalogActionHandle(name);
        });
    }
#endif
}

bool initialize()
{
#if !defined(BLEEP_WITH_STEAM)
    Corrade::Utility::Debug{} << "[SteamIntegration] Steam SDK disabled at build time.";
    return false;
#else
    if(gSteamInitialized)
        return gSteamInputInitialized;

    SteamErrMsg errMsg{};
    const ESteamAPIInitResult initResult = SteamAPI_InitEx(&errMsg);
    if(initResult != k_ESteamAPIInitResult_OK)
    {
        Corrade::Utility::Warning{}
            << "[SteamIntegration] SteamAPI_InitEx failed (" << int(initResult) << "):"
            << (errMsg[0] != '\0' ? errMsg : "Unknown Steam init error");
        logMissingInstall();
        return false;
    }

    gSteamInitialized = true;

    const bool manifestConfigured = configureManifest();

    if(!SteamInput()->Init(false))
    {
        Corrade::Utility::Warning{} << "[SteamIntegration] SteamInput init failed.";
        shutdown();
        return false;
    }

    if(manifestConfigured)
        loadActionHandles();
    else
        Corrade::Utility::Warning{} << "[SteamIntegration] Steam Input manifest missing, action handles not loaded.";
    gSteamInputInitialized = true;
    Corrade::Utility::Debug{} << "[SteamIntegration] SteamInput initialized.";
    return true;
#endif
}

void pump()
{
#if defined(BLEEP_WITH_STEAM)
    if(!gSteamInitialized)
        return;

    SteamAPI_RunCallbacks();

    if(gSteamInputInitialized)
        SteamInput()->RunFrame();
#endif
}

void shutdown()
{
#if defined(BLEEP_WITH_STEAM)
    if(gSteamInputInitialized)
    {
        SteamInput()->Shutdown();
        gSteamInputInitialized = false;
        Corrade::Utility::Debug{} << "[SteamIntegration] SteamInput shutdown.";
    }

    if(gSteamInitialized)
    {
        SteamAPI_Shutdown();
        gSteamInitialized = false;
        Corrade::Utility::Debug{} << "[SteamIntegration] SteamAPI shutdown.";
    }
#endif
}

bool isActive()
{
#if defined(BLEEP_WITH_STEAM)
    return gSteamInputInitialized;
#else
    return false;
#endif
}

const ActionHandles& handles()
{
    return gHandles;
}

const char* actionSetName(ActionSetId id)
{
    return ActionSetNames[static_cast<std::size_t>(id)];
}

const char* digitalActionName(DigitalActionId id)
{
    return DigitalActionNames[static_cast<std::size_t>(id)];
}

const char* analogActionName(AnalogActionId id)
{
    return AnalogActionNames[static_cast<std::size_t>(id)];
}

} // namespace SteamIntegration
