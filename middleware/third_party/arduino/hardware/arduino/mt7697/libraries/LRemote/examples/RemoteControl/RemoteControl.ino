/*
  This example configures LinkIt 7697 to a reciver of the iOS LinkIt Remote App

  created Aug 2017
*/
#include <LRemote.h>

LRemoteButton button;
LRemoteSlider slider;
LRemoteLabel label;
LRemoteSwitch switchButton;
LRemoteCircleButton bigButton;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  Serial.println("Start configuring remote");

  // Initialize GPIO
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  // Setup the Remote Control's UI canvas
  LRemote.setName("LinkIt 7697");
  LRemote.setOrientation(RC_PORTRAIT);
  LRemote.setGrid(3, 5);

  // Add a push button
  button.setText("Press Me");
  button.setPos(1, 1);
  button.setSize(2, 1);
  button.setColor(RC_PINK);
  LRemote.addControl(button);

  // Add a big, round button
  bigButton.setText("!BIG!");
  bigButton.setPos(0, 3);
  bigButton.setSize(3, 2);
  bigButton.setColor(RC_GREEN);
  LRemote.addControl(bigButton);

  // Add a slider
  slider.setText("Value Slider(-100 ~ 1024)");
  slider.setPos(0, 2);
  slider.setSize(3, 1);
  slider.setColor(RC_ORANGE);
  slider.setValueRange(-100, 1024, 0);
  LRemote.addControl(slider);

  // Add a simple text label
  label.setText("Remote Test");
  label.setPos(0, 0);
  label.setSize(3, 1);
  label.setColor(RC_GREY);
  LRemote.addControl(label);

  // Add an on/off switch
  switchButton.setText("USR LED");
  switchButton.setPos(0, 1);
  switchButton.setSize(1, 1);
  switchButton.setColor(RC_BLUE);
  LRemote.addControl(switchButton);

  // Start broadcasting our remote contoller
  // This method implicitly initialized underlying BLE subsystem
  // to create a BLE peripheral, and then
  // start advertisement on it.
  LRemote.begin();
  Serial.println("begin() returned");
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

  // Now we poll each control's status
  
  if(button.isValueChanged()){
    Serial.print("button new value =");
    Serial.println(button.getValue());
  }

  if(bigButton.isValueChanged()){
    Serial.print("big button new value =");
    Serial.println(bigButton.getValue());
  }

  if(switchButton.isValueChanged()){
    digitalWrite(LED_BUILTIN, switchButton.getValue());
    Serial.print("USR LED new value =");
    Serial.println(switchButton.getValue());
  }

  if(slider.isValueChanged()){
    Serial.print("slider to new value = ");
    Serial.println(slider.getValue());
  }
}