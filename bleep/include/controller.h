#ifndef controller_h
#define controller_h

#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Math/Vector2.h>
#include <SDL.h>

#include <string>

#if __APPLE__
#include <IOKit/hid/IOHIDLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>

static void activate_gamepad(__IOHIDDevice *device) {
  IOReturn r = IOHIDDeviceOpen(device, kIOHIDOptionsTypeNone);
  if (r != kIOReturnSuccess) {
    printf("  Failed to open device - %d\n", r);
    return;
  }
  uint8_t controlBlob[] = { 0x42, 0x0C, 0x00, 0x00};
  IOHIDDeviceSetReport(device, kIOHIDReportTypeFeature, 0xF4, controlBlob, sizeof(controlBlob));

  printf("  Activating device...\n");
  sleep(1);

  uint8_t rumbleBlob[] = {
    0x01,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // rumble values [0x00, right-timeout, right-force, left-timeout, left-force]
    0x00,
    0x00, // Gyro
    0x00,
    0x00,
    0x00, // 0x02=LED1 .. 0x10=LED4
    /*
     * the total time the led is active (0xff means forever)
     * |     duty_length: how long a cycle is in deciseconds:
     * |     |                              (0 means "blink very fast")
     * |     |     ??? (Maybe a phase shift or duty_length multiplier?)
     * |     |     |     % of duty_length led is off (0xff means 100%)
     * |     |     |     |     % of duty_length led is on (0xff is 100%)
     * |     |     |     |     |
     * 0xff, 0x27, 0x10, 0x00, 0x32,
     */
    0xff,
    0x27,
    0x10,
    0x00,
    0x32, // LED 4
    0xff,
    0x27,
    0x10,
    0x00,
    0x32, // LED 3
    0xff,
    0x27,
    0x10,
    0x00,
    0x32, // LED 2
    0xff,
    0x27,
    0x10,
    0x00,
    0x32, // LED 1
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    // Necessary for Fake DS3
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
  };
  static const size_t RumbleLengthL = 4;
  static const size_t RumblePowerL = 5;
  static const size_t RumbleLengthR = 2;
  static const size_t RumblePowerR = 3;
  rumbleBlob[RumbleLengthL] = rumbleBlob[RumbleLengthR] = 20;
  rumbleBlob[RumblePowerL]                              = 150;
  rumbleBlob[RumblePowerR]                              = 1;
  IOHIDDeviceSetReport(device, kIOHIDReportTypeOutput, 1, rumbleBlob, sizeof(rumbleBlob));
  IOHIDDeviceClose(device, kIOHIDOptionsTypeNone);
  printf("  Should be rumbling!\n");
}

void init_gamepad() {
  static const SInt32 VendorId = 0x054C;
  static const SInt32 ProductId = 0x0268;

  CFNumberRef vendorIdNum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &VendorId);
  CFNumberRef productIdNum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &ProductId);
  
  const void *keys[2] = {
    CFSTR(kIOHIDVendorIDKey),
    CFSTR(kIOHIDProductIDKey),
  };

  const void *values[2] = {
    vendorIdNum,
    productIdNum
  };

  IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);  
  CFDictionaryRef matching = CFDictionaryCreate(NULL, keys, values, 2, NULL, NULL);
  IOHIDManagerSetDeviceMatching(manager, matching);

  CFSetRef deviceSet = IOHIDManagerCopyDevices(manager);
  if (deviceSet != NULL) {
    CFIndex count = CFSetGetCount(deviceSet);
    if (count > 0) {
      printf("Discovered %ld DualShock 3 gamepads\n", count);
      __IOHIDDevice **gamepads = (__IOHIDDevice **)calloc(count, sizeof(__IOHIDDevice *));
      CFSetGetValues(deviceSet, (const void **)gamepads);
      for (CFIndex i = 0; i < count; i++) {
        printf("Handling device %ld:\n", i);
        activate_gamepad(gamepads[i]);
      }
      free(gamepads);
    } else {
      printf("No DualShock 3 gamepads found!\n");
    }
    CFRelease(deviceSet);
  }
  CFRelease(productIdNum);
  CFRelease(vendorIdNum);
  CFRelease(matching);
  CFRelease(manager);
}

#endif

using namespace Magnum;

#define MAX_NUM_JOYSTICKS 6

SDL_Joystick *joysticks[MAX_NUM_JOYSTICKS];
SDL_Haptic *haptics[MAX_NUM_JOYSTICKS];
int rumble[MAX_NUM_JOYSTICKS]; // 2 -> play rumble
int connected; // connected joysticks number

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
    
    ImGui::InvisibleButton("Spacer", ImVec2(1.0f, 100.0f)); // Creates 50px horizontal and 20px vertical space

    if (ImGui::Button("Search...")){
      #if __APPLE__
      init_gamepad();
      #endif
    };

    ImGui::End();
  }

  void update(){
    // leftJoystick = leftMovement.normalized();
    // rightJoystick = rightMovement.normalized();

    // if (isnan(leftJoystick.x())) leftJoystick.x() = 0;
    // if (isnan(leftJoystick.y())) leftJoystick.y() = 0;
    // if (isnan(rightJoystick.x())) rightJoystick.x() = 0;
    // if (isnan(rightJoystick.y())) rightJoystick.y() = 0;
  }

  void init(){
    // Initializing SDL with Joystick support (Joystick, game controller and haptic subsystems)
    if(SDL_Init( SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC ) < 0)
    {
      fprintf(stderr, "Error: Couldn't initialize SDL. %s\n", SDL_GetError());
      exit(1);
    }
    
    fprintf(stdout, "Joystick currently attached: %i.\n\n", SDL_NumJoysticks());
    if(SDL_NumJoysticks() == 0)
      fprintf(stdout, "Check the joystick is properly connected.\nIf the problem persist, you can check the connected USB device with 'lsusb' and the logs in /var/logs/syslog\n"); 
    
    SDL_JoystickEventState(SDL_ENABLE);
  }

  int getJoyIndex(int id)
  {
    int i;
    for(i = 0; i < MAX_NUM_JOYSTICKS; i++){
      printf(std::to_string(SDL_JoystickInstanceID(joysticks[0])).c_str());
      if(SDL_JoystickInstanceID(joysticks[i]) == id)
        return i;
    }
    return -1;
  }

  void doJoystickAxisMotion(SDL_JoyAxisEvent& event)
  {
    int i = getJoyIndex(event.which);
    
    if((event.value < -3200) || (event.value > 3200)) 
    {
      switch (event.axis){
        case 0:
          fprintf(stdout, "[%zu] SDL_JOYAXISMOTION\n  Joystick:\t#%i(%i)\n  Axis:\t\t%i (ANALOG_L)\n  Value:\t%i ", event.timestamp, i, event.which, event.axis, event.value);
          if(event.value < 0)
            fprintf(stdout, "(LEFT)\n\n");
          else
            fprintf(stdout, "(RIGHT)\n\n");
          leftJoystick.x() = event.value / 32767.0f;
            
          break;
        case 1:
          fprintf(stdout, "[%zu] SDL_JOYAXISMOTION\n  Joystick:\t#%i(%i)\n  Axis:\t\t%i (ANALOG_L)\n  Value:\t%i ", event.timestamp, i, event.which, event.axis, event.value);
          if(event.value < 0)
            fprintf(stdout, "(UP)\n\n");
          else
            fprintf(stdout, "(DOWN)\n\n");
          leftJoystick.y() = event.value / 32767.0f;

          break;
        case 2:
          fprintf(stdout, "[%zu] SDL_JOYAXISMOTION\n  Joystick:\t#%i(%i)\n  Axis:\t\t%i (TRIGGER_L)\n  Value:\t%i ", event.timestamp, i, event.which, event.axis, event.value);
          if(event.value < 0)
            fprintf(stdout, "(UP)\n\n");
          else
            fprintf(stdout, "(DOWN)\n\n");
          
            
          break;
        case 3:
          fprintf(stdout, "[%zu] SDL_JOYAXISMOTION\n  Joystick:\t#%i(%i)\n  Axis:\t\t%i (ANALOG_R)\n  Value:\t%i ", event.timestamp, i, event.which, event.axis, event.value);
          if(event.value < 0)
            fprintf(stdout, "(LEFT)\n\n");
          else
            fprintf(stdout, "(RIGHT)\n\n");
          rightJoystick.x() = event.value / 32767.0f;

          break;
        case 4:
          fprintf(stdout, "[%zu] SDL_JOYAXISMOTION\n  Joystick:\t#%i(%i)\n  Axis:\t\t%i (ANALOG_R)\n  Value:\t%i ", event.timestamp, i, event.which, event.axis, event.value);
          if(event.value < 0)
            fprintf(stdout, "(UP)\n\n");
          else
            fprintf(stdout, "(DOWN)\n\n");
          rightJoystick.y() = event.value / 32767.0f;

          break;
        case 5:
          fprintf(stdout, "[%zu] SDL_JOYAXISMOTION\n  Joystick:\t#%i(%i)\n  Axis:\t\t%i (TRIGGER_R)\n  Value:\t%i ", event.timestamp, i, event.which, event.axis, event.value);
          if(event.value < 0)
            fprintf(stdout, "(UP)\n\n");
          else
            fprintf(stdout, "(DOWN)\n\n");

          break;

        default:
          break;
      }
    }
  }

  void doJoystickButtonUp(SDL_JoyButtonEvent *event)
  {
    int i = getJoyIndex(event->which);
    fprintf(stdout, "[%zu] SDL_JOYBUTTONUP\n  Joystick:\t#%i(%i)\n  Button:\t%i\n\n", event->timestamp, i, event->which, event->button);
    
    
  }

  // void doJoystickButtonDown(SDL_JoyButtonEvent *event)
  // {
  //   int i = getJoyIndex(event->which);
  //   fprintf(stdout, "[%zu] SDL_JOYBUTTONDOWN\n  Joystick:\t#%i(%i)\n  Button:\t%i\n\n", event->timestamp, i, event->which, event->button);
    
  //   switch(event->button){
  //     case 2:
        

  //       break;

  //   }

  //   // rumble
  //   if(app.haptics[i] != NULL && (event->button == 6 || event->button == 7)) 
  //   {
  //     app.rumble[i]++;
  //     if(app.rumble[i] == 2)
  //       fprintf(stdout, "[%zu] RUMBLE_ON\n  Joystick:\t#%i(%i)\n\n", event->timestamp, i, event->which);
  //   }
  // }

  void doJoystickAdded(SDL_JoyDeviceEvent& event)
  {
    int i;
    
    // check if the array is full
    if(connected == MAX_NUM_JOYSTICKS)
    {
      fprintf(stdout, "Maximum number of connected joysticks reached.\n\n");
      return;
    }
    
    // search the first available index
    for(i = 0; i < MAX_NUM_JOYSTICKS; i++)
    {
      if(joysticks[i] == NULL) // found free spot
        break;
    }
    
    // joysticks and haptic devices opening
    fprintf(stdout, "[%zu] SDL_JOYDEVICEADDED\n", event.timestamp);
    if((joysticks[i] = SDL_JoystickOpen(event.which)) == NULL)
    {
      fprintf(stderr, "Error: Couldn't open the joystick(%i) SDL. %s\n\n", event.which, SDL_GetError());
      return;
    }
    connected++;
    
    fprintf(stdout, "  Index:\t\t#%i\n", i);
    fprintf(stdout, "  Joystick ID:\t\t%i\n", SDL_JoystickInstanceID(joysticks[i]));
    fprintf(stdout, "  Name:\t\t\t%s\n", SDL_JoystickName(joysticks[i]));
    fprintf(stdout, "  Number of Axes:\t%i\n", SDL_JoystickNumAxes(joysticks[i]));
    fprintf(stdout, "  Number of Buttons:\t%i\n", SDL_JoystickNumButtons(joysticks[i]));
    fprintf(stdout, "  Number of Balls:\t%i\n", SDL_JoystickNumBalls(joysticks[i]));
    
    // check if the joystick is a haptic device
    if(SDL_JoystickIsHaptic(joysticks[i]) < 0)
    {
      fprintf(stderr, "Error: Joystick #%i(%i) is not haptic, rumble can't be enabled. %s\n\n", i, event.which, SDL_GetError());
      return;
    }
    
    // haptic opening
    if((haptics[i] = SDL_HapticOpenFromJoystick(joysticks[i])) == NULL)
    {
      fprintf(stderr, "Error: Joystick #%i(%i) haptic opening failed. %s\n\n", i, event.which, SDL_GetError());
      return;
    }
    
    // check whether rumble is supported on a haptic device
    if(SDL_HapticRumbleSupported(haptics[i]) < 0)
    {
      fprintf(stderr, "Error: Joystick #%i(%i) doesn't support rumble. %s\n\n", i, event.which, SDL_GetError());
      SDL_HapticClose(haptics[i]);
      haptics[i] = NULL;
      return;
    }
    
    // Initialize a haptic device for simple rumble playback
    if(SDL_HapticRumbleInit(haptics[i]) < 0)
    {
      fprintf(stdout, "  Haptic rumble:\tdisabled\n\n");
      fprintf(stderr, "Error: Joystick #%i(%i) haptic rumble initialization failed. %s\n\n", i, event.which, SDL_GetError());
      SDL_HapticClose(haptics[i]);
      haptics[i] = NULL;
      return;
    }
    fprintf(stdout, "  Haptic rumble:\tenabled\n\n");
    
    fprintf(stdout, "Joystick currently connected: %i.\n\n", SDL_NumJoysticks());
  }

  void doJoystickRemoved(SDL_JoyDeviceEvent& event)
  {
    int i = getJoyIndex(event.which);
    
    fprintf(stdout, "[%zu] SDL_JOYDEVICEREMOVED\n", event.timestamp);
    
    // clear joystick
    SDL_JoystickClose(joysticks[i]);
    joysticks[i] = NULL;
    fprintf(stdout, "  Closed joystick:\t#%i(%i)\n", i, event.which);
    
    // clear haptic
    SDL_HapticClose(haptics[i]);
    haptics[i] = NULL;
    fprintf(stdout, "  Closed haptic device:\t#%i(%i)\n\n", i, event.which);
    
    rumble[i] = 0;
    connected--;
    fprintf(stdout, "Joystick currently connected: %i.\n\n", SDL_NumJoysticks());
  }

  float GetLeftJoystickScalar() {
    return sqrt(pow(leftJoystick.x(), 2) + pow(leftJoystick.y(), 2));
  }
  float GetRightJoystickScalar() {
    return sqrt(pow(rightJoystick.x(), 2) + pow(rightJoystick.y(), 2));
  }

  bool CheckIfJoysticksCentered(){
    // Checks if both joysticks are centered (or close enough to be considered centered)
    if (abs(leftJoystick.x()) <= 0.1 && abs(leftJoystick.y()) <= 0.1 && abs(rightJoystick.x()) <= 0.1 && abs(rightJoystick.y()) <= 0.1){
      leftJoystick.x() = 0;
      leftJoystick.y() = 0;
      rightJoystick.x() = 0;
      rightJoystick.y() = 0;              
      return true;
    } 
    else return false;
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