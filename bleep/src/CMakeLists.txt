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
main.cpp
steamIntegration.cpp
../include/MacSerialPort/SerialPort/SerialPort.cpp
    )

if(BLEEP_WITH_STEAM)
    set(STEAMWORKS_SDK_DIR "${PROJECT_SOURCE_DIR}/steam-sdk" CACHE PATH "Path to the Steamworks SDK root")
    target_include_directories(bleep PRIVATE "${STEAMWORKS_SDK_DIR}/public")
    set(STEAM_ACTION_MANIFEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/steam_input/steam_input_manifest.vdf")
    target_compile_definitions(bleep PRIVATE STEAM_ACTION_MANIFEST=\"${STEAM_ACTION_MANIFEST_PATH}\")

    if(APPLE)
        set(STEAMWORKS_LIB "${STEAMWORKS_SDK_DIR}/redistributable_bin/osx/libsteam_api.dylib")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(STEAMWORKS_LIB "${STEAMWORKS_SDK_DIR}/redistributable_bin/linux64/libsteam_api.so")
    elseif(WIN32)
        set(STEAMWORKS_LIB "${STEAMWORKS_SDK_DIR}/redistributable_bin/win64/steam_api64.lib")
    else()
        set(STEAMWORKS_LIB "")
    endif()

    if(STEAMWORKS_LIB AND EXISTS "${STEAMWORKS_LIB}")
        target_link_libraries(bleep PRIVATE "${STEAMWORKS_LIB}")
        if(APPLE)
            get_filename_component(STEAMWORKS_LIB_NAME "${STEAMWORKS_LIB}" NAME)
            add_custom_command(TARGET bleep POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${STEAMWORKS_LIB}"
                    "$<TARGET_FILE_DIR:bleep>/${STEAMWORKS_LIB_NAME}"
                COMMENT "Copying Steamworks dylib next to CLI executable")
        endif()
    else()
        message(WARNING "BLEEP_WITH_STEAM is ON but Steamworks library not found for this platform")
    endif()
endif()

if(BLEEP_WITH_STEAM)
    target_compile_definitions(bleep PRIVATE BLEEP_WITH_STEAM)
endif()


add_dependencies(bleep
    Magnum::AnyImageImporter
    Magnum::AnySceneImporter
    MagnumPlugins::AssimpImporter)

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

# if(CORRADE_TARGET_APPLE)
#     set_target_properties(bleep PROPERTIES
#         MACOSX_BUNDLE ON
#         MACOSX_BUNDLE_BUNDLE_NAME "My Application"
#         MACOSX_BUNDLE_BUNDLE_IDENTIFIER "cz.mosra.magnum.my-application")
# endif()

install(TARGETS bleep DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})

target_compile_definitions(bleep PRIVATE -DMODELS_DIR="${MODELS_DIR}")
