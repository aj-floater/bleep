#ifndef body_h
#define body_h

#include <Magnum/Math/Quaternion.h>
#include <Magnum/Math/Vector3.h>

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

#include "leg.h"

#include "controller.h"

using namespace Magnum;
using namespace Math::Literals;

class Body {
public:
  // Constructor
  Body() {
    _position = Vector3(0.0f, 0.25f, 0.0f);

    // Initialize legs using arrays
    for (int i = 0; i < numLegs; i++) {
      legs[i] = new Leg();
      initLeg(i);
    }
  }

  void initLeg(int i) {
    legs[i]->_position = _position + legOffsetPositions[i];
    legs[i]->_endPose = legDesiredPoses[i];
  }

  void updateLeg(int i){
    legs[i]->_position = _position + _rotation.transformVector(legOffsetPositions[i]);
    legs[i]->_rotation = _rotation;
  }

  void update(Float deltaTime) {
    HandleAnimation(deltaTime);

    // Update leg positions and rotations using arrays
    for (int i = 0; i < numLegs; i++) {
      updateLeg(i);
      legs[i]->update(deltaTime);
    }

  }

  void NewAnimation(Vector3 desiredPose, Quaternion desiredRotation, float speedConstant){
      this->_finalAnimationPose = desiredPose;
      this->_finalAnimationRotation = desiredRotation;

      // if ((this->_desiredPose - this->_endPose).length() < 0.2) return;

      this->_deltaVector = this->_finalAnimationPose - this->_position;
      this->_deltaQuaternion = this->_finalAnimationRotation - this->_rotation;
      
      this->_previousPosition = this->_position;
      this->_previousRotation = this->_rotation;
      this->_speed = _deltaVector.lengthInverted() * speedConstant;
      this->_rotationSpeed = 1/abs(Float(_deltaQuaternion.normalized().angle())) * speedConstant;

      this->_animationPlaying = true;
      this->_animationPosePlaying = true;
      this->_animationRotationPlaying = true;
  }

  void HandleAnimation(Float deltaTime){
      // if the distance from the desiredpose to the previouspose is greater than the distance to the currentendpose
      // then the endpose has not reached the desiredpose yet
      
      if (_animationPlaying){
          if ((this->_finalAnimationPose - this->_previousPosition).length() >= (this->_finalAnimationPose - this->_position).length()){
              // reset the previous end pose to the current end pose
              this->_previousPosition = this->_position;
              // update the endpose position so that it is always "infront" of the previouspose
              this->_position += this->_deltaVector * this->_speed * deltaTime;
          }
          else {
              this->_position = this->_finalAnimationPose;
              this->_animationPosePlaying = false;
          }

          if ((this->_finalAnimationRotation - this->_previousRotation).length() >= (this->_finalAnimationRotation - this->_rotation).length()){
            // reset the previous end pose to the current end pose
            this->_previousRotation = this->_rotation;
            // update the endpose position so that it is always "infront" of the previouspose
            _rotation = _rotation * _deltaQuaternion * _rotationSpeed * deltaTime;
          }
          else {
              this->_rotation = this->_finalAnimationRotation;
              this->_animationRotationPlaying = false;
          }

          if (!_animationRotationPlaying && !_animationPosePlaying) _animationPlaying = false;
      }
  }

  Quaternion changeRotation(Float x, Float y, Float z){
    Quaternion combinedRotation;
    combinedRotation = combinedRotation * Quaternion::rotation(Rad(x), Vector3::xAxis()); 
    combinedRotation = combinedRotation * Quaternion::rotation(Rad(y), Vector3::yAxis()); 
    combinedRotation = combinedRotation * Quaternion::rotation(Rad(z), Vector3::zAxis());

    return combinedRotation;
  }

  // Constants for leg positions
  static constexpr int numLegs = 6;
  Vector3 legOffsetPositions[numLegs] = {
    Vector3(0.60f, 0.0f, 0.86f),
    Vector3(0.60f, 0.0f, 0.0f),
    Vector3(0.60f, 0.0f, -0.86f),
    Vector3(-0.60f, 0.0f, 0.86f),
    Vector3(-0.60f, 0.0f, 0.0f),
    Vector3(-0.60f, 0.0f, -0.86f)
  };

  // Constants for leg desired end effector poses
  Vector3 legDesiredPoses[numLegs] = {
    Vector3(1.5f, 0.0f, 1.8f),
    Vector3(2.0f, 0.0f, 0.0f),
    Vector3(1.5f, 0.0f, -1.8f),
    Vector3(-1.5f, 0.0f, 1.8f),
    Vector3(-2.0f, 0.0f, 0.0f),
    Vector3(-1.5f, 0.0f, -1.8f)
  };

  // Array to hold leg instances
  Leg* legs[numLegs];

  Controller* controllerPointer;

  Quaternion _rotation;

  bool _animationPlaying = false;
  bool _animationPosePlaying = false;
  bool _animationRotationPlaying = false;
  Float _speed;
  Float _rotationSpeed;
  Vector3 _position;
  Vector3 _previousPosition;
  Quaternion _previousRotation;

  Vector3 _travelVector;
  Vector3 _finalAnimationPose;
  Quaternion _finalAnimationRotation;
  Vector3 _desiredPose;
  Vector3 _deltaVector;
  Quaternion _deltaQuaternion;
};

#endif