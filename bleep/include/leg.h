#ifndef leg_h
#define leg_h

#include <Magnum/Timeline.h>
#include "joint.h"

#include <Magnum/Math/Quaternion.h>
#include <Magnum/Math/Vector3.h>

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

using namespace Magnum;
using namespace Math::Literals;

// Forward declarations
extern SceneGraph::DrawableGroup3D _drawables;
extern Scene3D _scene;

class Leg {
public:
    // Constructor
    Leg() {
        // Initialize features of base joint
        BaseJoint = new Joint(Vector3::yAxis(), 0.48f);
        // Initialize features of second joint
        BaseJoint->addChild(Vector3::zAxis(), 0.6f);
        SecondJoint = BaseJoint->_child;
        // Initialize features of third joint
        SecondJoint->addChild(Vector3::zAxis(), 1.3f);
        ThirdJoint = SecondJoint->_child;

        // BaseJoint->_length = 0.48f;

        // SecondJoint->_length = 0.6f;
        // SecondJoint->_axisofrotation = Vector3::zAxis();

        // ThirdJoint->_length = 1.3f;
        // ThirdJoint->_axisofrotation = Vector3::zAxis();
    }

    // Function to update the leg
    void update(float deltaTime) {
        HandleAnimation(deltaTime);

        // Calculate the inverse kinematics
        if (showIK)
            CalculateIK();

        // Propagate throughout all joints in the leg
        // to calculate the forward kinematics given the individual joint angles
        BaseJoint->update(_position, _rotation);
    }

    void CalculateIK(){
        Vector3 startPosition = _position;
        Vector3 endPosition = _endPose;

        Quaternion _invertedRotation = _rotation.inverted();
        endPosition = _invertedRotation.transformVector(endPosition);
        startPosition = _invertedRotation.transformVector(startPosition);
        
        Vector3 deltaPosition = endPosition - startPosition;


        Float angle_1 = atan2f(deltaPosition.z(), deltaPosition.x());

        Float x = (endPosition.z()/sinf(angle_1)) - ((startPosition.z()/sinf(angle_1)) + BaseJoint->_length);
        if (startPosition.z() == endPosition.z())
            x = (endPosition.x()/cosf(angle_1)) - ((startPosition.x()/cosf(angle_1)) + BaseJoint->_length);

        Float y = - deltaPosition.y();

        Float l1 = SecondJoint->_length;
        Float l2 = ThirdJoint->_length;

        // Calculate angle 2
        Float angle_3 = acosf((- powf(l1, 2) - powf(l2, 2) + powf(x, 2) + powf(y, 2))/(2*l1*l2));
        
        // Calculate angle 1
        Float angle_2 = atan2f(y,x) - atan2f((l2*sinf(angle_3)),(l1+l2*cosf(angle_3)));

        if (!std::isnan(angle_1) && !std::isnan(angle_2) && !std::isnan(angle_3)){
            SecondJoint->_angle = - angle_2;
            ThirdJoint->_angle = - angle_3;
        }
        BaseJoint->_angle = - angle_1;
    }

    void NewAnimation(){
        if (!this->_animationPlaying){
            this->_deltaVector = this->_desiredPose - this->_endPose;
            
            this->_previousEndPose = this->_endPose;
            this->_speedConstant = _deltaVector.lengthInverted() * this->_speed;

            this->_animationPlaying = true;
        }
        else {
            // Debug{} << "New Animation was not created";
        }
    }
    void NewAnimation(Vector3 desiredPose){
        if (!this->_animationPlaying){
            this->_desiredPose = desiredPose;
            this->_deltaVector = this->_desiredPose - this->_endPose;
            this->_animationPlaying = true;
        }
        else {
            // Debug{} << "New Animation was not created";
        }
    }

    void HandleAnimation(Float deltaTime){
        // if the distance from the desiredpose to the previouspose is greater than the distance to the currentendpose
        // then the endpose has not reached the desiredpose yet
        
        if (_animationPlaying){
            if ((this->_desiredPose - this->_previousEndPose).length() >= (this->_desiredPose - this->_endPose).length()){
                // reset the previous end pose to the current end pose
                this->_previousEndPose = this->_endPose;
                // update the endpose position so that it is always "infront" of the previouspose
                this->_endPose += this->_deltaVector * this->_speedConstant * deltaTime;
            }
            else {
                this->_endPose = this->_desiredPose;
                this->_animationPlaying = false;
            }
        }
    }

    // Member variables
    Joint* BaseJoint;
    Joint* SecondJoint;
    Joint* ThirdJoint;

    Joint* DesiredJoint;

    bool _animationPlaying = false;
    Float _speed = 0.5f;
    Float _speedConstant = 0.3f;
    Vector3 _endPose;
    Vector3 _previousEndPose;

    Vector3 _travelVector;
    Vector3 _desiredPose;
    Vector3 _deltaVector;

    Quaternion _rotation;
    Vector3 _position;

    bool showIK = true;
};

#endif