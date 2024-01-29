#include <Magnum/Timeline.h>

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/ImGuiIntegration/Context.hpp>

#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>
#include <Magnum/SceneGraph/AbstractTranslationRotationScaling3D.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>

#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Grid.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Math/Color.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Trade/MeshData.h>

#include "ArcBall.h"
#include "ArcBallCamera.h"

#include "controller.h"
#include "cubeDrawable.h"
#include "meshDrawable.h"
#include "graphicsBody.h"

using namespace Magnum;
using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::TranslationRotationScalingTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::TranslationRotationScalingTransformation3D> Scene3D;
class GridDrawable: public SceneGraph::Drawable3D {
  public:
    explicit GridDrawable(Object3D& object, SceneGraph::DrawableGroup3D* group, int subdivisions, Color3 color):
      SceneGraph::Drawable3D{object, group},
      _color(color)
    {
      _mesh = MeshTools::compile(Primitives::grid3DWireframe(Vector2i(subdivisions)));
    }

  private:
    void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override {
      using namespace Math::Literals;

      _shader.setAmbientColor(_color)
        .setTransformationMatrix(transformationMatrix)
        .setNormalMatrix(transformationMatrix.normalMatrix())
        .setProjectionMatrix(camera.projectionMatrix())
        .draw(_mesh);
    }

    GL::Mesh _mesh;
    Shaders::PhongGL _shader;
    Color3 _color;
};

Object3D* grid;

Color3 color;

SceneGraph::DrawableGroup3D _drawables;
Scene3D _scene;

bool playing = false;

class MyApplication: public Platform::Application {
public:
  explicit MyApplication(const Arguments& arguments);

  void viewportEvent(ViewportEvent& event) override;

  void keyPressEvent(KeyEvent& event) override;
  void keyReleaseEvent(KeyEvent& event) override;

  void mousePressEvent(MouseEvent& event) override;
  void mouseReleaseEvent(MouseEvent& event) override;
  void mouseMoveEvent(MouseMoveEvent& event) override;
  void mouseScrollEvent(MouseScrollEvent& event) override;
  void textInputEvent(TextInputEvent& event) override;

  Timeline _timeline;

private:
  ImGuiIntegration::Context _imgui{NoCreate};

  void renderGUI();
  void drawEvent() override;
  

  Vector2i _lastPosition;

  Containers::Optional<ArcBallCamera> _arcballCamera;
  GraphicsLeg* debuggingLeg;
  GraphicsBody* body;
  Cube* cube;
  Controller* controller;
};

MyApplication::MyApplication(const Arguments& arguments):
  Platform::Application{arguments, Configuration{}
    .setTitle("Bleep")
    .setWindowFlags(Configuration::WindowFlag::Resizable)}
{
  using namespace Math::Literals;

  _imgui = ImGuiIntegration::Context(Vector2{windowSize()}/dpiScaling(), windowSize(), framebufferSize());

  GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
  GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
    GL::Renderer::BlendEquation::Add);
  GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
    GL::Renderer::BlendFunction::OneMinusSourceAlpha);

  /* Configure camera */
  {
    /* Setup the arcball after the camera objects */
    const Vector3 eye = Vector3::zAxis(-10.0f);
    const Vector3 center{};
    const Vector3 up = Vector3::yAxis();
    _arcballCamera.emplace(_scene, eye, center, up, 45.0_degf,
      windowSize(), framebufferSize());
  }

  /* TODO: Prepare your objects here and add them to the scene */
  grid = new Object3D{&_scene};
  (*grid)
    // .translate(Vector3::yAxis(-0.3f))
    .rotateX(90.0_degf)
    .translate(Vector3(0.5f, 0.0f, 0.5f))
    .scale(Vector3(20.0f));
  new GridDrawable{*grid, &_drawables, 40, Color3(0.3f, 0.3f, 0.3f)};

  controller = new Controller();

  debuggingLeg = new GraphicsLeg();
  debuggingLeg->showDebuggingWindow = true;
  debuggingLeg->showMeshes = true;
  debuggingLeg->_position = Vector3(5.0f, 0.0f, 0.0f);
  debuggingLeg->_desiredPose = Vector3(6.5f, 0.0f, 0.8f);
  debuggingLeg->_endPose = Vector3(7.0f, 0.0f, 0.0f);

  body = new GraphicsBody(Color3(0.75f, 0.75f, 0.75f));

  body->controllerPointer = controller;

  setMinimalLoopPeriod(16);

  _timeline.start();
}

bool showLegInfoWindow = false; // A flag to track whether to show the leg info window or not

void MyApplication::renderGUI() {
  _imgui.newFrame();

  /* Enable text input, if needed */
  if(ImGui::GetIO().WantTextInput && !isTextInputActive())
      startTextInput();
  else if(!ImGui::GetIO().WantTextInput && isTextInputActive())
      stopTextInput();

  
  // ImGui::ShowDemoWindow();

  // ImGuiIO& io = ImGui::GetIO();
  if(ImGui::BeginMainMenuBar()){
    if(ImGui::BeginMenu("Scene Debugging")){
      ImGui::SeparatorText("Grid");
      {
        Float (&translation)[3] = grid->translation().data();
        if (ImGui::DragFloat3("Grid Translation", translation, 0.1f)){
          grid->setTranslation(Vector3::from(translation));
        };
      }
      ImGui::SeparatorText("Debugging Leg");
      if (ImGui::RadioButton("Show leg", debuggingLeg->showDebuggingWindow))
        debuggingLeg->showDebuggingWindow = !debuggingLeg->showDebuggingWindow;

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  { // Leg Debugging
    ImGui::Begin("Leg Debugging");
    
    if (ImGui::RadioButton("Play", playing)){
      playing = !playing;
    };

    ImGui::End();
  }

  debuggingLeg->showDebugging();

  body->showGUI();

  controller->showGUI();

  /* Update application cursor */
  _imgui.updateApplicationCursor(*this);

  /* Set appropriate states. If you only draw ImGui, it is sufficient to
      just enable blending and scissor debuggingLeg in the constructor. */
  GL::Renderer::enable(GL::Renderer::Feature::Blending);
  GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
  GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
  GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

  _imgui.drawFrame();

  /* Reset state. Only needed if you want to draw something else with
      different state after. */
  GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
  GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
  GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
  GL::Renderer::disable(GL::Renderer::Feature::Blending);
}



void MyApplication::drawEvent() {
  GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

  controller->update();

  {
    Float speed = 0.3f;
    if ((debuggingLeg->_desiredPose - debuggingLeg->_endPose).length() < (debuggingLeg->_desiredPose - debuggingLeg->_previousEndPose).length()){
      debuggingLeg->_deltaVector = debuggingLeg->_desiredPose - debuggingLeg->_endPose;
      Debug{} << "Found";
    }

    debuggingLeg->_endPose += debuggingLeg->_deltaVector * speed * _timeline.currentFrameDuration();

    debuggingLeg->_previousEndPose = debuggingLeg->_endPose;
  }

  debuggingLeg->update();

  body->update(_timeline.currentFrameDuration());

  _arcballCamera->update();
    _arcballCamera->draw(_drawables);

  renderGUI();

  swapBuffers();
  redraw();
  _timeline.nextFrame();
}

void MyApplication::viewportEvent(ViewportEvent& event) {
  GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

  _arcballCamera->reshape(event.windowSize(), event.framebufferSize());

  _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
      event.windowSize(), event.framebufferSize());
}

void MyApplication::keyPressEvent(KeyEvent& event) {
    if (_imgui.handleKeyPressEvent(event))
        return;

    switch (event.key()) {
        case KeyEvent::Key::W:
            controller->leftMovement.y() = -1.0f;
            break;
        case KeyEvent::Key::S:
            controller->leftMovement.y() = 1.0f;
            break;
        case KeyEvent::Key::A:
            controller->leftMovement.x() = -1.0f;
            break;
        case KeyEvent::Key::D:
            controller->leftMovement.x() = 1.0f;
            break;
        case KeyEvent::Key::Up:
            controller->rightMovement.y() = -1.0f;
            break;
        case KeyEvent::Key::Down:
            controller->rightMovement.y() = 1.0f;
            break;
        case KeyEvent::Key::Left:
            controller->rightMovement.x() = -1.0f;
            break;
        case KeyEvent::Key::Right:
            controller->rightMovement.x() = 1.0f;
            break;
        default:
            break;
    }
}

void MyApplication::keyReleaseEvent(KeyEvent& event) {
    if (_imgui.handleKeyReleaseEvent(event))
        return;

    switch (event.key()) {
        case KeyEvent::Key::W:
        case KeyEvent::Key::S:
            controller->leftMovement.y() = 0.0f;
            break;
        case KeyEvent::Key::A:
        case KeyEvent::Key::D:
            controller->leftMovement.x() = 0.0f;
            break;
        case KeyEvent::Key::Up:
        case KeyEvent::Key::Down:
            controller->rightMovement.y() = 0.0f;
            break;
        case KeyEvent::Key::Left:
        case KeyEvent::Key::Right:
            controller->rightMovement.x() = 0.0f;
            break;
        default:
            break;
    }
}

void MyApplication::mousePressEvent(MouseEvent& event) {
    if (_imgui.handleMousePressEvent(event)) return;
    /* Enable mouse capture so the mouse can drag outside of the window */
    /** @todo replace once https://github.com/mosra/magnum/pull/419 is in */
    SDL_CaptureMouse(SDL_TRUE);

    _arcballCamera->initTransformation(event.position());

    event.setAccepted();
    redraw(); /* camera has changed, redraw! */
}

void MyApplication::mouseReleaseEvent(MouseEvent& event) {
    if (_imgui.handleMouseReleaseEvent(event)) return;
    /* Disable mouse capture again */
    /** @todo replace once https://github.com/mosra/magnum/pull/419 is in */

    SDL_CaptureMouse(SDL_FALSE);
}

void MyApplication::mouseMoveEvent(MouseMoveEvent& event) {
    if (_imgui.handleMouseMoveEvent(event)) return;

    if(!event.buttons()) return;

    if (event.buttons() == MouseMoveEvent::Button::Right) {
        _arcballCamera->translate(event.position());
    } else if (event.modifiers() & MouseMoveEvent::Modifier::Shift) {
        _arcballCamera->translate(event.position());
    } else {
        _arcballCamera->rotate(event.position());
    }

    event.setAccepted();
    redraw(); /* camera has changed, redraw! */
}

void MyApplication::mouseScrollEvent(MouseScrollEvent& event) {
    if (_imgui.handleMouseScrollEvent(event)) return;

    const Float delta = event.offset().y();
    if(Math::abs(delta) < 1.0e-2f) return;

    _arcballCamera->zoom(delta);

    event.setAccepted();
    redraw(); /* camera has changed, redraw! */
}

void MyApplication::textInputEvent(TextInputEvent& event) {
    if(_imgui.handleTextInputEvent(event)) return;
}

MAGNUM_APPLICATION_MAIN(MyApplication)