/*
  This example configures LinkIt 7697 to a reciver of the iOS LinkIt Remote App

  created Aug 2017
*/
#include <LRemote.h>

LRemoteJoyStick stickLeft;
LRemoteJoyStick stickRight;
LRemoteLabel labelLeft;
LRemoteLabel labelRight;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  Serial.println("Start configuring remote");

  // Initialize GPIO
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  // Setup the Remote Control's UI canvas
  LRemote.setName("LinkIt 7697");
  
  // We use a landscape since it is easir to contorl joysticks
  LRemote.setOrientation(RC_LANDSCAPE);
  LRemote.setGrid(4, 2);

  // Add left stick
  stickLeft.setPos(0, 1);
  stickLeft.setSize(1, 1);
  stickLeft.setColor(RC_ORANGE);
  LRemote.addControl(stickLeft);

  // Add Right stick
  stickRight.setPos(3, 1);
  stickRight.setSize(1, 1);
  stickRight.setColor(RC_BLUE);
  LRemote.addControl(stickRight);

    // Add a simple text label
  labelLeft.setText("(0, 0)");
  labelLeft.setPos(0, 0);
  labelLeft.setSize(1, 1);
  labelLeft.setColor(RC_GREY);
  LRemote.addControl(labelLeft);

  labelRight.setText("(0, 0)");
  labelRight.setPos(3, 0);
  labelRight.setSize(1, 1);
  labelRight.setColor(RC_GREY);
  LRemote.addControl(labelRight);


  // Start broadcasting our remote contoller
  // This method implicitly initialized underlying BLE subsystem
  // to create a BLE peripheral, and then
  // start advertisement on it.
  LRemote.begin();
  Serial.println("begin() returned");
}

void checkAndUpdateLabel(LRemoteLabel& label, LRemoteJoyStick& stick) {
  if(stick.isValueChanged()){
    LRemoteDirection d = stick.getValue();
    // d.x and d.y are the value from the Joystick component:
    // d.x : -100 ~ 100, where 0 is center, -100 is leftmost, and 100 is rightmost.
    // d.y : -100 ~ 100, where 0 is center, -100 is bottommost, and 100 is topmost.

    // you can print d directly.
    Serial.println(d);

    // in this example, we simply pass the value of d.x/d.y
    // back to the LinkIt Remote app.
    label.updateText(d.toString());
  } 
}

void loop() {
  // check if we are connect by some 
  // BLE central device, e.g. an mobile app
  if(!LRemote.connected()) {
    Serial.println("waiting for connection");
    delay(1000);
  } else {
    // The interval between button down/up
    // can be very short - e.g. a quick tap
    // on the screen.
    // We could lose some event if we
    // delay something like 100ms.
    delay(15);
  }
  
  // Process the incoming BLE write request
  // and translate them to control events
  LRemote.process();

  // refer to this function to know how to parse the values from joystick.
  checkAndUpdateLabel(labelLeft, stickLeft);
  checkAndUpdateLabel(labelRight, stickRight);
  
}