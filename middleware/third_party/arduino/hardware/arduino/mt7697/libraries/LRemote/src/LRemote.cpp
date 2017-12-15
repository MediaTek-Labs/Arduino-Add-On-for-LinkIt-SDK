#include <Arduino.h>
#include <LBLE.h>
#include <LBLEPeriphral.h>
#include "LRemote.h"
#include <vector>

LRemoteClass LRemote;

// UUID definition
static LBLEUuid rcServiceUUID("3f60ab39-1710-4456-930c-7e9c9539917e");
static LBLEService rcService(rcServiceUUID);
static LBLECharacteristicInt
    rcControlCount("3f60ab39-1711-4456-930c-7e9c9539917e");
static LBLECharacteristicBuffer
    rcControlTypes("3f60ab39-1712-4456-930c-7e9c9539917e");
static LBLECharacteristicInt rcRow("3f60ab39-1713-4456-930c-7e9c9539917e");
static LBLECharacteristicInt rcCol("3f60ab39-1714-4456-930c-7e9c9539917e");
static LBLECharacteristicBuffer
    rcColors("3f60ab39-1715-4456-930c-7e9c9539917e"); // Array of UINT8, enum of
                                                      // ColorType of each
                                                      // control
static LBLECharacteristicBuffer
    rcFrames("3f60ab39-1716-4456-930c-7e9c9539917e"); // Array of UINT8[4], (x,
                                                      // y, row, col) of each
                                                      // control
static LBLECharacteristicString
    rcNames("3f60ab39-1717-4456-930c-7e9c9539917e"); // String of control names,
                                                     // separated by the \n
                                                     // ASCII character
static LBLECharacteristicBuffer
    rcEvent("b5d2ff7b-6eff-4fb5-9b72-6b9cff5181e7"); // Array of UINT8[4],
                                                     // (sequence, event,
                                                     //  event data_byte1,
                                                     //  event data_byte2,
                                                     // #) of each control

static LBLECharacteristicBuffer
    rcUIUpdate("e4b1ddfe-eb37-4c78-aba8-c5fa944775cb"); // Of variable-length type RCUIUpdateInfo

                                                     
static LBLECharacteristicBuffer
    rcConfigDataArray("5d7a63ff-4155-4c7c-a348-1c0a323a6383");

static LBLECharacteristicInt rcOrientation(
    "203fbbcd-9967-4eba-b0ff-0f72e5a634eb"); // 0: portrait, 1: landscape

static LBLECharacteristicInt rcProtocolVersion("ae73266e-65d4-4023-8868-88b070d5d576"); // Incremental

void LRemoteClass::begin() {
  // Initialize BLE subsystem if necessary
  initBLEOnce();

  // configure our advertisement data.
  // In this case, we simply create an advertisement that represents an
  // connectable device with a device name
  LBLEAdvertisementData advertisement;

  // since we broadcast service UUID,
  // this leaves us little room for device name in the advertisement packet.
  String deviceName = m_deviceName;
  // truncate to fit into advertisement packet
  deviceName.remove(8);
  advertisement.configAsConnectableDevice(deviceName.c_str(), rcServiceUUID);

  // Configure our device's Generic Access Profile's device name
  // Ususally this is the same as the name in the advertisement data.
  LBLEPeripheral.setName(m_deviceName.c_str());

  // Set default values
  const size_t count = m_controls.size();
  rcProtocolVersion.setValue(PROTOCOL_VERSION);
  rcControlCount.setValue(count);
  rcCol.setValue(m_canvasColumn);
  rcRow.setValue(m_canvasRow);
  rcOrientation.setValue(m_orientation);

  LBLEValueBuffer typeArray;
  typeArray.resize(count);

  LBLEValueBuffer colorArray;
  colorArray.resize(count);

  LBLEValueBuffer frameArray;
  frameArray.resize(count * 4); // (x, y, r, c) are 4 uint8_t

  RCEventInfo eventInfo;
  eventInfo.seq = 0;
  eventInfo.controlIndex = 0;
  eventInfo.event = 0;
  eventInfo.processedSeq = 0;
  eventInfo.data = 0;

  LBLEValueBuffer configDataArray;
  configDataArray.resize(count * sizeof(RCConfigData));

  // for each control, we populate
  // all the corresponding information such as
  // color, control type, label text, event and other configuration data.
  for (int i = 0; i < count; i++) {
    // initialize control index
    m_controls[i]->m_controlIndex = i;

    // setup BLE attributes
    typeArray[i] = (uint8_t)m_controls[i]->m_type;
    colorArray[i] = (uint8_t)m_controls[i]->m_color;
    frameArray[i * 4 + 0] = m_controls[i]->m_x;
    frameArray[i * 4 + 1] = m_controls[i]->m_y;
    frameArray[i * 4 + 2] = m_controls[i]->m_row;
    frameArray[i * 4 + 3] = m_controls[i]->m_col;

    // fill the additional config data
    RCConfigData configData;
    m_controls[i]->fillConfigData(configData);
    *((RCConfigData *)&configDataArray[i * sizeof(RCConfigData)]) = configData;
  }

  // text label are updated separately,
  // and can be updated after begin() is called
  updateTextLabel();

  rcControlTypes.setValueBuffer(&typeArray[0], typeArray.size());
  rcColors.setValueBuffer(&colorArray[0], colorArray.size());
  rcFrames.setValueBuffer(&frameArray[0], frameArray.size());
  rcEvent.setValueBuffer((uint8_t*)&eventInfo, sizeof(eventInfo));
  rcConfigDataArray.setValueBuffer(&configDataArray[0], configDataArray.size());

  RCUIUpdateInfo dummyUpdateInfo;
  memset(&dummyUpdateInfo, 0, sizeof(dummyUpdateInfo));
  rcUIUpdate.setValueBuffer((uint8_t*)&dummyUpdateInfo, sizeof(dummyUpdateInfo));

  initPeripheralOnce();

  // start advertisment
  LBLEPeripheral.advertise(advertisement);
}

void LRemoteClass::updateTextLabel() {
  const size_t count = m_controls.size();
  String nameList;
  for (int i = 0; i < count; i++) {
    nameList += m_controls[i]->m_text;
    nameList += '\n';
  }
  rcNames.setValue(nameList);
}

void LRemoteClass::initBLEOnce() {
  if (m_bleInitialized) {
    return;
  }

  m_bleInitialized = true;

  // Initialize BLE subsystem
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }

  // Add characteristics into rcService
  rcService.addAttribute(rcProtocolVersion);
  rcService.addAttribute(rcCol);
  rcService.addAttribute(rcRow);
  rcService.addAttribute(rcControlCount);
  rcService.addAttribute(rcControlTypes);
  rcService.addAttribute(rcColors);
  rcService.addAttribute(rcFrames);
  rcService.addAttribute(rcNames);
  rcService.addAttribute(rcEvent);
  rcService.addAttribute(rcUIUpdate);
  rcService.addAttribute(rcConfigDataArray);
  rcService.addAttribute(rcOrientation);

  // Add service to GATT server (peripheral)
  LBLEPeripheral.addService(rcService);
}

void LRemoteClass::initPeripheralOnce() {
  if (m_peripheralBegan) {
    return;
  }

  // start the GATT server - it is now
  // available to connect
  LBLEPeripheral.begin();

  m_peripheralBegan = true;
}

bool LRemoteClass::connected() { return LBLEPeripheral.connected(); }

void LRemoteClass::process() {
  // check if new event coming in
  if (rcEvent.isWritten()) {
    const LBLECharacteristicWrittenInfo &written = rcEvent.getLastWrittenInfo();

    RCEventInfo event = {0};

    // retrieve the control's event info
    rcEvent.getValue((uint8_t *)&event, sizeof(event), 0);

    // scan and update event info
    // make sure that the "sequence number" is incremented
    // (and thus different from the control's record)
    const size_t i = event.controlIndex;
    LRemoteUIControl &control = *m_controls[i];
    const uint8_t oldSeq = control.m_lastEvent.seq;
    control.m_lastEvent = event;
    control.m_lastEvent.seq = oldSeq + 1;

    event.processedSeq = event.seq;
    rcEvent.setValueBuffer((uint8_t *)&event, sizeof(event));
  }
}

void LRemoteClass::end() {
  // force disconnect all clients and stop advertise ourselves.
  // Note: the underlying BT subssystem is still kept alive.
  LBLEPeripheral.disconnectAll();
  LBLEPeripheral.stopAdvertise();
}

///////////////////////////////////////////////////////
// UI Control classes
//////////////////////////////////////////////////////
void LRemoteLabel::updateText(const String &text) {
  
  // This comes from experimenting on Android 6.0.1
  // iOS 11 does not have such limiation
  const int ANDROID_GATT_NOTIFICATION_LIMIT = 15;

  // input check, make sure it is shorter than 15 bytes
  const String verifiedText = text.length() > ANDROID_GATT_NOTIFICATION_LIMIT ? text.substring(0, ANDROID_GATT_NOTIFICATION_LIMIT) : text;
  setText(verifiedText);

  // send notification
  std::vector<uint8_t> updateInfoBuffer(verifiedText.length() + 1 + sizeof(RCUIUpdateInfo));
  RCUIUpdateInfo *pInfo = reinterpret_cast<RCUIUpdateInfo*>(&updateInfoBuffer[0]);
  pInfo->controlIndex = m_controlIndex;
  pInfo->dataSize = verifiedText.length() + 1;
  memcpy(pInfo->data, verifiedText.c_str(), verifiedText.length() + 1);

  rcUIUpdate.setValueBuffer(&updateInfoBuffer[0], updateInfoBuffer.size());

  if (LBLEPeripheral.connected()) {
    LBLEPeripheral.notifyAll(rcUIUpdate);
  }

  // update default value, which is read during connection / re-connection
  LRemote.updateTextLabel();
}