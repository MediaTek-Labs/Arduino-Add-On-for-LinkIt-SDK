/*
  This example configures LinkIt 7697 to a reciver of the iOS LinkIt Remote App

  created Aug 2017
*/
#include <LRemote.h>


LRemoteUIControl btn;
LRemoteUIControl slider;
LRemoteUIControl label;
LRemoteUIControl switchButton;
LRemoteUIControl bigButton;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // Initialize BLE subsystem
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }

  // Setup the Remote Control's UI canvas
  LRemote.setGrid(3, 5);
  LRemote.setName("Demo Con");

  // Add a push button
  btn.setType(RC_PUSHBUTTON);
  btn.setText("OK");
  btn.setPos(1, 1);
  btn.setSize(2, 1);
  btn.setColor(RC_PINK);
  LRemote.addControl(btn);

  // Add a big, round button
  bigButton.setType(RC_CIRCLEBUTTON);
  bigButton.setText("OK");
  bigButton.setPos(0, 3);
  bigButton.setSize(3, 2);
  bigButton.setColor(RC_GREEN);
  LRemote.addControl(bigButton);

  // Add a slider
  slider.setType(RC_SLIDER);
  slider.setText("MinMax");
  slider.setPos(0, 2);
  slider.setSize(3, 1);
  slider.setColor(RC_GOLD);
  LRemote.addControl(slider);

  // Add a simple text label
  label.setType(RC_LABEL);
  label.setText("Remote Test");
  label.setPos(0, 0);
  label.setSize(3, 1);
  label.setColor(RC_GREY);
  LRemote.addControl(label);

  // Add an on/off switch
  switchButton.setType(RC_SWITCHBUTTON);
  switchButton.setText("Power");
  switchButton.setPos(0, 1);
  switchButton.setSize(1, 1);
  switchButton.setColor(RC_BLUE);
  LRemote.addControl(switchButton);

  // Start broadcasting our remote contoller
  LRemote.begin();
}

void loop() {

  if(!LBLEPeripheral.connected()) {
    Serial.println("waiting for connection");
    delay(1000);
  } else {
    delay(100);
  }
  
  // Process the incoming BLE write request
  // and translate them to control events
  LRemote.process();

  // Now we poll each control's status
  
  if(btn.hasEvent()){
    RCEventInfo info = btn.getLastEvent();
    Serial.print("btn event=");
    switch(info.event) {
      case RC_BTNUP:
        Serial.println("BTN_UP");
        break;
      case RC_BTNDOWN:
        Serial.println("BTN_DOWN");
    }
  }

  if(bigButton.hasEvent()){
    RCEventInfo info = bigButton.getLastEvent();
    Serial.print("bigButton event=");
    switch(info.event) {
      case RC_BTNUP:
        Serial.println("BTN_UP");
        break;
      case RC_BTNDOWN:
        Serial.println("BTN_DOWN");
    }
  }

  if(switchButton.hasEvent()){
    RCEventInfo info = switchButton.getLastEvent();
    Serial.print("switch to new value = ");
    Serial.println(switchButton.getValue());
  }

  if(slider.hasEvent()){
    RCEventInfo info = slider.getLastEvent();
    Serial.print("slider to new value = ");
    Serial.println(slider.getValue());
  }
}