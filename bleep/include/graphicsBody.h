#ifndef graphics_body_h
#define graphics_body_h

#include "body.h"
#include "graphicsLeg.h"

#include <Magnum/Math/Color.h>
#include <Magnum/ImGuiIntegration/Context.hpp>

using namespace Magnum;
using namespace Math::Literals;

// Forward declarations
extern SceneGraph::DrawableGroup3D _drawables;
extern Scene3D _scene;

#define MOVEMENT 0
#define ROTATION 1

// Create GraphicsBody class that inherits from Body
class GraphicsBody : public Body {
public:
  // Constructor
  GraphicsBody(Color3 color) : Body() {
    _color = color;
    _scale = Vector3(10.1f);

    _position = Vector3(0.0f, 0.25f, 0.0f);

    // Initialize legs using arrays
    for (int i = 0; i < numLegs; i++) {
      legs[i] = new GraphicsLeg();
      initLeg(i);
    }

    initMeshDrawObject(std::string(MODELS_DIR) + "/body.stl");
    _meshrotation = Quaternion::rotation(-90.0_degf, Vector3::xAxis());
  }

  void initLeg(int i) {
    legs[i]->_position = _position + legOffsetPositions[i];
    legs[i]->_endPose = legDesiredPoses[i];
    legs[i]->_desiredPose = legDesiredPoses[i];
  }

  void updateLeg(int i){
    legs[i]->_position = _position + _rotation.transformVector(legOffsetPositions[i]);
    legs[i]->_rotation = _rotation;
  }

  void update(Float deltaTime) {
    {
      updatePhantomBody(); // update the phantom body's position and rotation (as well as each legs d)

      // Loop through the legs and animate them accordingly
      // ------------------------------------------------------------------------------------------------------------------------
      // Calculate desired positions of the first set based off currentPosition using the phantomPosition and phantomRotation
      // Store the phantom as deltaPosition and deltaRotation - in delta -
      
      // Move first set to calculated desired positions

      // Set toggle to 2
      // Loop   --------
      // Calculate desired positions of the "toggle" set based off the delta and new phantom

      // Move "toggle" set to calculated desired positions
      
      // Move and rotate the body to the delta

      // Store the phantom - in delta -

      // Toggle the toggle
      // --------
      // ------------------------------------------------------------------------------------------------------------------------

      HandleAnimation(deltaTime);
    }

    // Update leg positions and rotations using arrays
    for (int i = 0; i < numLegs; i++) {
        updateLeg(i);
        legs[i]->update(deltaTime);
    }

    // Update the draw object for visualization
    updateDrawObject();
  }

  void updatePhantomBody(){
    _phantomPosition = Vector3(controllerPointer->leftJoystick.x() + _position.x(), 0, controllerPointer->leftJoystick.y() + _position.z());
    _phantomRotation = _rotation.rotation(Rad(-0.4 * controllerPointer->rightJoystick.x()), Vector3::yAxis());
    for (int i = 0; i < numLegs; i++) {
      legs[i]->_desiredPose = _phantomRotation.transformVector(_phantomPosition + legDesiredPoses[i]);
    }
  }

  // Function to initialize the MeshDrawable for visualization
  void initMeshDrawObject(std::string meshFile) {
    _meshobject = new Object3D{&_scene};
    (*_meshobject)
      .setRotation(_rotation)
      .setTranslation(_position)
      .scale(_scale);

    // Create the MeshDrawable with the specified meshFile
    _meshdrawable = new MeshDrawable{*_meshobject, &_drawables, &_color, meshFile};
  }
  
  // Function to update the visual representation of the joint
  void updateDrawObject() {
    (*_meshobject)
      .setRotation(_rotation * _meshrotation)
      .setTranslation(_position + _rotation.transformVector(_meshposition))
      .setScaling(_scale);
  }

  float rotation[3] = {0.0f, 0.0f, 0.0f};
  float position[3] = {0.0f, 0.0f, 0.0f};

  void showBodyControl(){
    ImGui::Begin("Body Control");

    ImGui::SeparatorText("Body Position/Rotation");
    ImGui::PushItemWidth(120);
    
    position[0] = _position.x();
    position[1] = _position.y();
    position[2] = _position.z();
    if (ImGui::DragFloat3("Position - Body", position, 0.01f)){
    _position = Vector3(
        position[0],
        position[1],
        position[2]
    );
    }

    if(ImGui::DragFloat3("Rotation - Body", rotation, 0.01f)){
      Quaternion combinedRotation;
      combinedRotation = combinedRotation * Quaternion::rotation(Rad(rotation[0]), Vector3::xAxis()); 
      combinedRotation = combinedRotation * Quaternion::rotation(Rad(rotation[1]), Vector3::yAxis()); 
      combinedRotation = combinedRotation * Quaternion::rotation(Rad(rotation[2]), Vector3::zAxis());

      _rotation = combinedRotation;
    }
    

    ImGui::SeparatorText("Modes");
    bool isModeMovement = (mode == MOVEMENT) ? true : false; 
    if (ImGui::RadioButton("Movement - Mode", isModeMovement)){
      isModeMovement = !isModeMovement;
      if (isModeMovement)
        mode = MOVEMENT;
    }
    bool isModeRotation = (mode == ROTATION) ? true : false; 
    if (ImGui::RadioButton("Rotation - Mode", isModeRotation)){
      isModeRotation = !isModeRotation;
      if (isModeRotation)
        mode = ROTATION;
    }

    ImGui::End();
  }

  void showGUI(){
    showBodyControl();
  }

private:
  GraphicsLeg* legs[numLegs];

  Vector3 _scale;
  Color3 _color;

  // Graphics-related members
  Object3D* _meshobject;
  MeshDrawable* _meshdrawable;
  Vector3 _meshposition;
  Quaternion _meshrotation;

  

  bool showDebuggingWindow = false;
  int mode = ROTATION;
};

#endif