/*
  This example configures LinkIt 7697 to a reciver of the iOS LinkIt Remote App

  created Aug 2017
*/
#include <LRemote.h>


LRemoteUIControl btn;
LRemoteUIControl slider;
LRemoteUIControl label;
LRemoteUIControl switchButton;


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // Initialize BLE subsystem
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }

  LRemote.setGrid(3, 5);
  LRemote.setName("Demo Con");

  btn.setType(RC_PUSHBUTTON);
  btn.setText("OK");
  btn.setPos(1, 1);
  btn.setSize(2, 1);
  btn.setColor(RC_PINK);
  LRemote.addControl(btn);

  slider.setType(RC_SLIDER);
  slider.setText("MinMax");
  slider.setPos(0, 2);
  slider.setSize(3, 1);
  slider.setColor(RC_GOLD);
  LRemote.addControl(slider);

  label.setType(RC_LABEL);
  label.setText("Remote Test");
  label.setPos(0, 0);
  label.setSize(3, 1);
  label.setColor(RC_GREY);
  LRemote.addControl(label);

  switchButton.setType(RC_SWITCHBUTTON);
  switchButton.setText("Power");
  switchButton.setPos(0, 1);
  switchButton.setSize(1, 1);
  switchButton.setColor(RC_GREEN);
  LRemote.addControl(switchButton);

  LRemote.begin();
}

void loop() {

  if(!LBLEPeripheral.connected()) {
    Serial.println("waiting for connection");
    delay(1000);
  } else {
    delay(100);
  }
  
  LRemote.process();

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