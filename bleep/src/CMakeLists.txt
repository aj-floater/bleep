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

find_package(Eigen3 REQUIRED)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

add_executable(bleep

ArcBall.cpp
main.cpp
    )


add_dependencies(bleep
    Magnum::AnyImageImporter
    Magnum::AnySceneImporter
    MagnumPlugins::AssimpImporter)

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
    MagnumIntegration::ImGui)

install(TARGETS bleep DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})

target_compile_definitions(bleep PRIVATE -DMODELS_DIR="${MODELS_DIR}")