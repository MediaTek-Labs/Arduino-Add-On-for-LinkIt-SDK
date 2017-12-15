/*
  This example configures LinkIt 7697 to a reciver of the iOS LinkIt Remote App

  created Aug 2017
*/
#include <LRemote.h>

LRemoteButton buttonLeft;
LRemoteButton buttonRight;
LRemoteLabel labelLeft;
LRemoteLabel labelRight;
int counterLeft = 0;
int counterRight = 0;


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
  LRemote.setGrid(2, 4);

  // Add a simple text label
  labelLeft.setText(String(counterLeft, 10));
  labelLeft.setPos(0, 1);
  labelLeft.setSize(1, 1);
  labelLeft.setColor(RC_GREY);
  LRemote.addControl(labelLeft);

  labelRight.setText(String(counterRight, 10));
  labelRight.setPos(1, 1);
  labelRight.setSize(1, 1);
  labelRight.setColor(RC_GREY);
  LRemote.addControl(labelRight);

  // Add two push button
  buttonLeft.setText("Add Left");
  buttonLeft.setPos(0, 2);
  buttonLeft.setSize(1, 1);
  buttonLeft.setColor(RC_PINK);
  LRemote.addControl(buttonLeft);

  buttonRight.setText("Add Right");
  buttonRight.setPos(1, 2);
  buttonRight.setSize(1, 1);
  buttonRight.setColor(RC_PINK);
  LRemote.addControl(buttonRight);

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

  // Increase the label counter when button is released.
  if(buttonLeft.isValueChanged() && !buttonLeft.getValue()){
    counterLeft++;
    // you can send at most 15 bytes of characters.
    labelLeft.updateText(String(counterLeft, 10));
    Serial.println("increase left counter");
  }

  // Increase the label counter when button is released.
  if(buttonRight.isValueChanged() && !buttonRight.getValue()){
    counterRight++;
    labelRight.updateText(String(counterRight, 10));
    Serial.println("increase right counter");
  }  
}