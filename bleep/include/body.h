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
    // Update leg positions and rotations using arrays
    for (int i = 0; i < numLegs; i++) {
      updateLeg(i);
      legs[i]->update(deltaTime);
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
  Vector3 _position;
};

#endif