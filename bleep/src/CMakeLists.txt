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

add_executable(bleep MACOSX_BUNDLE

ArcBall.cpp
main.cpp
steamIntegration.cpp
../include/MacSerialPort/SerialPort/SerialPort.cpp
    )

if(BLEEP_WITH_STEAM)
    set(STEAMWORKS_SDK_DIR "${PROJECT_SOURCE_DIR}/steam-sdk" CACHE PATH "Path to the Steamworks SDK root")
    target_include_directories(bleep PRIVATE "${STEAMWORKS_SDK_DIR}/public")
    set(STEAM_APPID_SOURCE "${PROJECT_SOURCE_DIR}/steam_appid.txt")
    set(STEAM_INPUT_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../steam_input/steam_input_manifest.vdf"
        "${CMAKE_CURRENT_SOURCE_DIR}/../steam_input/xbox_controller.vdf"
        "${CMAKE_CURRENT_SOURCE_DIR}/../steam_input/steam_controller.vdf")

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
        get_filename_component(STEAMWORKS_LIB_NAME "${STEAMWORKS_LIB}" NAME)
        add_custom_command(TARGET bleep POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:bleep>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${STEAMWORKS_LIB}"
                "$<TARGET_FILE_DIR:bleep>/${STEAMWORKS_LIB_NAME}"
            COMMENT "Copying Steamworks dylib next to executable")
    else()
        message(WARNING "BLEEP_WITH_STEAM is ON but Steamworks library not found for this platform")
    endif()

    foreach(steam_input_file IN LISTS STEAM_INPUT_FILES)
        get_filename_component(_steam_input_name "${steam_input_file}" NAME)
        add_custom_command(TARGET bleep POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:bleep>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${steam_input_file}"
                "$<TARGET_FILE_DIR:bleep>/${_steam_input_name}"
            COMMENT "Copying ${_steam_input_name} next to executable")
    endforeach()

    if(EXISTS "${STEAM_APPID_SOURCE}")
        add_custom_command(TARGET bleep POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:bleep>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${STEAM_APPID_SOURCE}"
                "$<TARGET_FILE_DIR:bleep>/steam_appid.txt"
            COMMENT "Copying steam_appid.txt next to executable")
    else()
        message(WARNING "steam_appid.txt not found; Steam may fail to start the app")
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
target_compile_definitions(bleep PRIVATE BLEEP_DEFAULT_LOG_PATH=\"${CMAKE_BINARY_DIR}/log\")
