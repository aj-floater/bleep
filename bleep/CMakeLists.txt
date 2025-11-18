# Set the path to your models directory
set(MODELS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/models")

# Copy the contents of the models directory to the build directory during build
file(COPY ${MODELS_DIR} DESTINATION ${CMAKE_BINARY_DIR})

set(STEAM_INPUT_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/steam_input_manifest.vdf")
if(EXISTS "${STEAM_INPUT_MANIFEST}")
    configure_file(${STEAM_INPUT_MANIFEST} ${CMAKE_BINARY_DIR}/steam_input_manifest.vdf COPYONLY)
endif()

set(STEAM_APPID_DEFAULT 480 CACHE STRING "Default Steam App ID for local builds")

include_directories(include)

add_subdirectory(src)
