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
      updatePhantomBody(); // update the phantom body's position and rotation (as well as each legs desiredPose's)

      // Loop through the legs according to the gaitorder array and animate them accordingly
      // ------------------------------------------------------------------------------------------------------------------------
      int legFinishedAnimatingCount = 0; // a counter that counts how many legs have finished animating
      for (int i = 0; i < 6; i++){
        // 1. First we check if the leg is labelled as one that should be moved
        if (gaitorder[i] == gaittoggle){
          // 2. If it is then check if it already has an animation applied to it
          if (!legs[i]->_animationPlaying)
            legs[i]->NewAnimation(); // 3. If not then apply it
          // 4. If it has then mark it in the gait order as a leg that has been animated (or is being animated)
          else gaitorder[i] = 2;
        }
        // 5. Now we know that when the leg is marked and the animation is no longer playing, the animation must have finished
        //    so we increment the counter
        if (!legs[i]->_animationPlaying && gaitorder[i] == 2){
          legFinishedAnimatingCount++;
        }
      }
      // 6. When all three legs have finished animating we can reset the gait order and invert the gaittoggle for the next set of legs
      if (legFinishedAnimatingCount >= 3){
        for (int i = 0; i < 6; i++){
          if (gaitorder[i] == 2) gaitorder[i] = gaittoggle;
        }

        if (gaittoggle == 0) gaittoggle = 1; else gaittoggle = 0;
      }
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

  int gaittoggle = 0;
  int gaitorder[6] =  { 0, 1,
                        0, 1, 
                        0, 1 };

  bool showDebuggingWindow = false;
  int mode = ROTATION;
};

#endif