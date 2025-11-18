set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/magnum/modules/" ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/magnum-integration/modules/" ${CMAKE_MODULE_PATH})

find_package(Corrade REQUIRED Main)
find_package(Magnum REQUIRED
    GL
    MeshTools
    Primitives
    SceneGraph
    Shaders
    Trade
    Sdl2Application)
find_package(MagnumIntegration REQUIRED ImGui)

find_package(MagnumPlugins REQUIRED AssimpImporter)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

add_executable(bleep

ArcBall.cpp
SteamIntegration.cpp
main.cpp
../include/MacSerialPort/SerialPort/SerialPort.cpp
    )


add_dependencies(bleep
    Magnum::AnyImageImporter
    Magnum::AnySceneImporter
    MagnumPlugins::AssimpImporter)

if(BLEEP_ENABLE_STEAM)
    set(BLEEP_STEAM_INPUT_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/../steam_input_manifest.vdf")
    if(EXISTS "${BLEEP_STEAM_INPUT_MANIFEST}")
        target_compile_definitions(bleep PRIVATE BLEEP_STEAM_INPUT_MANIFEST_PATH="${BLEEP_STEAM_INPUT_MANIFEST}")
    endif()

    if(NOT EXISTS "${STEAMWORKS_SDK_INCLUDE_DIR}/steam/steam_api.h")
        message(FATAL_ERROR "Steamworks SDK headers not found at ${STEAMWORKS_SDK_INCLUDE_DIR}. Adjust STEAMWORKS_SDK_INCLUDE_DIR.")
    endif()
    target_include_directories(bleep PRIVATE "${STEAMWORKS_SDK_INCLUDE_DIR}")
endif()

if(APPLE)
    find_library(CoreFoundation CoreFoundation)
    find_library(IOKit IOKit)
    target_link_libraries(bleep PRIVATE 
        "-framework CoreFoundation -framework IOKit"
        Corrade::Main
        Magnum::Application
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Trade
        MagnumIntegration::ImGui
        )
elseif()
    target_link_libraries(bleep PRIVATE 
    Corrade::Main
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::SceneGraph
    Magnum::Shaders
    Magnum::Trade
    MagnumIntegration::ImGui
    )
endif()

if(BLEEP_ENABLE_STEAM)
    set(_BLEEP_STEAM_SDK_LIB "")
    if(APPLE)
        set(_BLEEP_STEAM_SDK_LIB "${STEAMWORKS_SDK_LIB_DIR}/libsteam_api.dylib")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(_BLEEP_STEAM_SDK_LIB "${STEAMWORKS_SDK_LIB_DIR}/libsteam_api.so")
    elseif(WIN32)
        set(_BLEEP_STEAM_SDK_LIB "${STEAMWORKS_SDK_LIB_DIR}/steam_api64.lib")
    endif()

    if(_BLEEP_STEAM_SDK_LIB STREQUAL "")
        message(WARNING "Steamworks SDK library path not set for this platform; Bleep will build without linking SteamInput.")
    elseif(NOT EXISTS "${_BLEEP_STEAM_SDK_LIB}")
        message(WARNING "Steamworks SDK library ${_BLEEP_STEAM_SDK_LIB} not found; Bleep will build without linking SteamInput.")
    else()
        target_link_libraries(bleep PRIVATE "${_BLEEP_STEAM_SDK_LIB}")
        target_compile_definitions(bleep PRIVATE BLEEP_WITH_STEAM=1)
        add_custom_command(TARGET bleep POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_BLEEP_STEAM_SDK_LIB}"
                "$<TARGET_FILE_DIR:bleep>"
            COMMENT "Copying Steam runtime library beside bleep executable")
        if(STEAM_APPID_DEFAULT)
            add_custom_command(TARGET bleep POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "${STEAM_APPID_DEFAULT}" > "$<TARGET_FILE_DIR:bleep>/steam_appid.txt"
                COMMENT "Writing default steam_appid.txt (${STEAM_APPID_DEFAULT})")
        endif()
    endif()
endif()

# if(CORRADE_TARGET_APPLE)
#     set_target_properties(bleep PROPERTIES
#         MACOSX_BUNDLE ON
#         MACOSX_BUNDLE_BUNDLE_NAME "My Application"
#         MACOSX_BUNDLE_BUNDLE_IDENTIFIER "cz.mosra.magnum.my-application")
# endif()

install(TARGETS bleep DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})

target_compile_definitions(bleep PRIVATE -DMODELS_DIR="${MODELS_DIR}")
