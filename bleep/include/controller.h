#ifndef controller_h
#define controller_h

#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Math/Vector2.h>

using namespace Magnum;

class Controller {
public:
  Controller() : leftJoystick{0.0f}, rightJoystick{0.0f} {}

  void DrawJoystick(float x, float y, ImVec2 position, float radius) {
    if (isnan(x)) x = 0;
    if (isnan(y)) y = 0;

    // Draw the outer circle of the joystick
    ImGui::GetWindowDrawList()->AddCircleFilled(position, radius, ImGui::GetColorU32(ImGuiCol_Button), 12);

    // Calculate the position of the inner circle based on the joystick's input
    ImVec2 joystickPos(position.x + (x * radius * 0.6), position.y + (y * radius * 0.6));

    // Draw the inner circle of the joystick
    ImGui::GetWindowDrawList()->AddCircleFilled(joystickPos, radius * 0.6f, ImGui::GetColorU32(ImGuiCol_ButtonActive), 12);
}

  void showGUI(){
    ImGui::Begin("Joystick Controller");

    ImVec2 position = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth()/2 - 50, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()/2);
    DrawJoystick(leftJoystick.x(), leftJoystick.y(), position, 40);

    position = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth()/2 + 50, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()/2);
    DrawJoystick(rightJoystick.x(), rightJoystick.y(), position, 40);

    ImGui::End();
  }

  void update(){
    leftJoystick = leftMovement.normalized();
    rightJoystick = rightMovement.normalized();

    if (isnan(leftJoystick.x())) leftJoystick.x() = 0;
    if (isnan(leftJoystick.y())) leftJoystick.y() = 0;
    if (isnan(rightJoystick.x())) rightJoystick.x() = 0;
    if (isnan(rightJoystick.y())) rightJoystick.y() = 0;
  }

  void CheckIfJoysticksCentered(){
    // Checks if both joysticks are centered (or close enough to be considered centered)
    if (abs(leftJoystick.x()) <= 0.1 && abs(leftJoystick.y()) <= 0.1 && abs(rightJoystick.x()) <= 0.1 && abs(rightJoystick.y()) <= 0.1){
      centered = true;
    } 
    else if (centered == true){
      centered = false;
    }
  }

  Vector2 leftJoystick;
  Vector2 rightJoystick;
  Vector2 leftMovement;
  Vector2 rightMovement;

  bool centered = false;

  bool leftbutton;
  bool rightbutton;
};

#endif