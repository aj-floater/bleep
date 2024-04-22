#ifndef graphics_body_h
#define graphics_body_h

#include "body.h"
#include "graphicsLeg.h"

#include <Magnum/Math/Color.h>
#include <Magnum/ImGuiIntegration/Context.hpp>

using namespace Magnum;
using namespace Math::Literals;
// using namespace Math;

// Forward declarations
extern SceneGraph::DrawableGroup3D _drawables;
extern Scene3D _scene;

#define MOVEMENT 0
#define ROTATION 1

#define FIRST 1
#define SECOND 2
#define THIRD 3

#define IDLE 0
#define SCHEDULED 1
#define ENGAGED 2

class Phase {
public:
  int first; // Encoded status 
  int second; // Encoded status 
  int third; // Encoded status 

  // Default constructor that initializes name and status to default values
  Phase() : first(IDLE), second(IDLE), third(IDLE) {}  // Common convention for default values
  // Function to get a human-readable string for the status
  static const char* GetStatusString(int status) {
    switch (status) {
      case IDLE: return "Idle";
      case SCHEDULED: return "Scheduled";
      case ENGAGED: return "Engaged";
      default: return "Unknown";
    }
  }
};

// Create GraphicsBody class that inherits from Body
class GraphicsBody : public Body {
public:
  // Constructor
  GraphicsBody(Color3 color) : Body() {
    _color = color;
    _scale = Vector3(10.1f);

    _position = Vector3(0.0f, 0.8f, 0.0f);

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
    // legs[i]->_desiredPose = legDesiredPoses[i];
  }

  void updateLeg(int i){
    legs[i]->_position = _position + _rotation.transformVector(legOffsetPositions[i]);
    legs[i]->_rotation = _rotation;
  }

  bool CheckIfAnimationsFinished(){
    for (int i = 0; i < 6; i++){
      // Debug{} << legs[i]->_animationPlaying;
      if (legs[i]->_animationPlaying){
        return false;
      }
    }
    // if (_animationPlaying) return false;

    return true;
  }

  bool joysticksCentered = false;

  Phase phase;

  Quaternion deltaRotation;
  Vector3 deltaPosition;
  
  Quaternion phantomRotation;
  Vector3 phantomPosition;

  void update(Float deltaTime) {
    HandleAnimation(deltaTime);

    {

      if (controllerPointer->CheckIfJoysticksCentered()){
        joysticksCentered = true;
        // reset
        if (phase.first == SCHEDULED || phase.second == SCHEDULED){
          phase.first = IDLE;
          phase.second = IDLE;
          phase.third = SCHEDULED;
        }
      }
      else if (phase.first == IDLE && phase.second == IDLE && phase.third == IDLE){
        phase.first = SCHEDULED;
      }

      if (phase.first != IDLE || phase.second != IDLE || phase.third != IDLE) {
        joysticksCentered = false;

        // First Phase --------------
        if (phase.first == SCHEDULED){
          // Calculate desired positions of the first set based off currentPosition using the phantomPosition and phantomRotation
          // Store the phantom as deltaPosition and deltaRotation - in delta -
          float rotationMultiplier = 2 * asinf(_stepSize / 4.0f);
          deltaRotation = Quaternion::rotation(Rad(-rotationMultiplier * controllerPointer->rightJoystick.x()), Vector3::yAxis());
          deltaPosition = Vector3(
            controllerPointer->leftJoystick.x() * _stepSize, 
            0, 
            controllerPointer->leftJoystick.y() * _stepSize
          );
          // the idea is to treat the new rotation from the controller seperately and add it to the previous rotation
          deltaRotation = deltaRotation * _rotation;
          // while transforming the position using the new rotation and adding the current position (ignoring the height the body is off the ground)
          deltaPosition = deltaRotation.transformVector(deltaPosition) + Vector3(_position.x(), 0, _position.z());
          // Move first set to calculated desired positions
          for (int i = 0; i < 6; i++){
            if (_gaitOrder[i] == _gaitToggle){
              // apply the new rotation to the desired leg positions
              // then add the transformed position
              legs[i]->NewAnimation( deltaRotation.transformVector(legDesiredPoses[i]) + deltaPosition );
            }
          }
          phase.first = ENGAGED;
        }
        
        // Second Phase -------------
        if (phase.second == SCHEDULED){
          // Loop   --------
          // Calculate desired positions of the "toggle" set based off the delta and new phantom
          float rotationMultiplier = 2 * asinf(_stepSize / 4.0f);
          phantomRotation = Quaternion::rotation(Rad(-rotationMultiplier * controllerPointer->rightJoystick.x()), Vector3::yAxis());
          phantomPosition = Vector3(
            controllerPointer->leftJoystick.x() * _stepSize, 
            0, 
            controllerPointer->leftJoystick.y() * _stepSize
          );
          phantomPosition = (_rotation * phantomRotation).transformVector(phantomPosition);
          // Move "toggle" set to calculated desired positions
          for (int i = 0; i < 6; i++){
            if (_gaitOrder[i] == _gaitToggle){
              legs[i]->NewAnimation( deltaRotation.transformVector(legDesiredPoses[i]) + deltaPosition + phantomPosition);
            }
          }

          Math::Vector3<Rad> eulerAngles = phantomRotation.normalized().toEuler();
          eulerAngles[1] = eulerAngles[1] / 2;
          Quaternion a =
            Quaternion::rotation(eulerAngles[2], Vector3::zAxis())*
            Quaternion::rotation(eulerAngles[1], Vector3::yAxis())*
            Quaternion::rotation(eulerAngles[0], Vector3::xAxis());
          this->NewAnimation(Vector3(deltaPosition.x() + phantomPosition.x()/2, _position.y(), deltaPosition.z() + phantomPosition.z()/2), deltaRotation);

          phase.second = ENGAGED;
        }
        // Third Phase -------------
        if (phase.third == SCHEDULED){
          Vector3 _groundPosition = Vector3(
            _position.x(),
            0,
            _position.z()
          );
          for (int i = 0; i < 6; i++){
            if (_gaitOrder[i] == _gaitToggle){
              legs[i]->NewAnimation( _rotation.transformVector(legDesiredPoses[i]) + _groundPosition );
            }
          }

          phase.third = ENGAGED;
        }
        
        { // Phase Handler
          int counter = 0;
          for (int i = 0; i < 6; i++){
            if (legs[i]->_animationPlaying)
              counter++;
          }
          if (counter == 0){
            if (phase.first == ENGAGED){
              phase.second = SCHEDULED;
              phase.first = IDLE;

              _gaitToggle = 2;
            }
            if (phase.second == ENGAGED){
              phase.second = SCHEDULED;

              // _position = Vector3(deltaPosition.x() + phantomPosition.x()/2, _position.y(), deltaPosition.z() + phantomPosition.z()/2);
              deltaPosition += phantomPosition;

              deltaRotation = deltaRotation * phantomRotation;

              if (_gaitToggle==1) _gaitToggle = 2;
              else if (_gaitToggle==2) _gaitToggle = 1;
            }
            if (phase.third == ENGAGED){
              Vector3 _groundPosition = Vector3(
                _position.x(),
                0,
                _position.z()
              );
              counter = 0;
              for (int i = 0; i < 6; i++){
                Vector3 desiredPosition = _rotation.transformVector(legDesiredPoses[i]) + _groundPosition;
                if ((legs[i]->_endPose - desiredPosition).length() < 0.05f)
                  counter++;
              }
              if (counter <= 5){
                phase.third = SCHEDULED;

                if (_gaitToggle==1) _gaitToggle = 2;
                else if (_gaitToggle==2) _gaitToggle = 1;
              }
              else phase.third = IDLE;
            }
          }
        }
      }
    }


    // Update leg positions and rotations using arrays
    for (int i = 0; i < numLegs; i++) {
        updateLeg(i);
        legs[i]->update(deltaTime);
    }

    // Update the draw object for visualization
    updateDrawObject();
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

    ImGui::SeparatorText("Control");
    ImGui::DragFloat("Step Time", &_stepTime, 0.01f);

    ImGui::DragFloat("Step Size", &_stepSize, 0.01f);

    ImGui::End();
  }

  void showPhases() {
    // Create a window
    ImGui::Begin("Phase Status");

    // Temporary array to store status colors
    ImVec4 statusColors[4];
    statusColors[IDLE] = ImVec4(1.0f, 0.65f, 0.0f, 1.0f); // Orange
    statusColors[SCHEDULED] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
    statusColors[ENGAGED] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
    statusColors[3] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White (Unknown)

    // Display the status of each phase with color
    for (int i = 0; i < 3; ++i) {
      const char* phaseName = (i == 0) ? "Phase 1" : (i == 1) ? "Phase 2" : "Phase 3";
      int status = (i == 0) ? phase.first : (i == 1) ? phase.second : phase.third;
      ImGui::PushStyleColor(ImGuiCol_Text, statusColors[status]);
      ImGui::Text("%s: %s", phaseName, Phase::GetStatusString(status));
      ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    ImGui::Text("Gait Toggle: %i", _gaitToggle);

    ImGui::SeparatorText("Leg Animation Status");
    for (int i = 0; i < 6; i++){
      ImGui::Text("Leg %i: %s", i, boolToString(legs[i]->_animationPlaying).c_str());
    }

    // End the window
    ImGui::End();
  }

  std::string boolToString(bool value) {
      return value ? "true" : "false";
  }

  void showGUI(){
    showBodyControl();
    showPhases();
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