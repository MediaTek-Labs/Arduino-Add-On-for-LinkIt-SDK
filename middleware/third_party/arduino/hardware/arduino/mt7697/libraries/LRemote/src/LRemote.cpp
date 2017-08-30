#include <Arduino.h>
#include <LBLE.h>
#include <LBLEPeriphral.h>

#include "LRemote.h"

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
    rcEventArray("b5d2ff7b-6eff-4fb5-9b72-6b9cff5181e7"); // Array of UINT8[4],
                                                          // (event, event data,
                                                          // reserved, sequence
                                                          // #) of each control

void LRemoteClass::begin() {
  // configure our advertisement data.
  // In this case, we simply create an advertisement that represents an
  // connectable device with a device name
  LBLEAdvertisementData advertisement;

  // since we broadcast service UUID,
  // this leaves us little room for device name in the advertisement packet.
  String deviceName = m_deviceName;
  deviceName.remove(9); // truncate to fit into advertisement packet
  advertisement.configAsConnectableDevice(deviceName.c_str(), rcServiceUUID);

  // Configure our device's Generic Access Profile's device name
  // Ususally this is the same as the name in the advertisement data.
  LBLEPeripheral.setName(m_deviceName.c_str());

  // Add characteristics into rcService
  rcService.addAttribute(rcCol);
  rcService.addAttribute(rcRow);
  rcService.addAttribute(rcControlCount);
  rcService.addAttribute(rcControlTypes);
  rcService.addAttribute(rcColors);
  rcService.addAttribute(rcFrames);
  rcService.addAttribute(rcNames);
  rcService.addAttribute(rcEventArray);

  // Add service to GATT server (peripheral)
  LBLEPeripheral.addService(rcService);

  // Set default values
  const size_t count = m_controls.size();
  rcControlCount.setValue(count);
  rcCol.setValue(m_canvasColumn);
  rcRow.setValue(m_canvasRow);

  LBLEValueBuffer typeArray;
  typeArray.resize(count);
  LBLEValueBuffer colorArray;
  colorArray.resize(count);
  LBLEValueBuffer frameArray;
  frameArray.resize(count * 4); // (x, y, r, c) are 4 uint8_t
  LBLEValueBuffer eventArray;
  eventArray.resize(count * 4);
  String nameList;

  for (int i = 0; i < count; i++) {
    typeArray[i] = (uint8_t)m_controls[i]->m_type;
    colorArray[i] = (uint8_t)m_controls[i]->m_color;
    frameArray[i * 4 + 0] = m_controls[i]->m_x;
    frameArray[i * 4 + 1] = m_controls[i]->m_y;
    frameArray[i * 4 + 2] = m_controls[i]->m_row;
    frameArray[i * 4 + 3] = m_controls[i]->m_col;
    nameList += m_controls[i]->m_text;
    nameList += '\n';
  }

  rcControlTypes.setValueBuffer(&typeArray[0], typeArray.size());
  rcColors.setValueBuffer(&colorArray[0], colorArray.size());
  rcFrames.setValueBuffer(&frameArray[0], frameArray.size());
  rcNames.setValue(nameList);

  rcEventArray.setValueBuffer(&eventArray[0], eventArray.size());

  // start the GATT server - it is now
  // available to connect
  LBLEPeripheral.begin();

  // start advertisment
  LBLEPeripheral.advertise(advertisement);
}

void LRemoteClass::process() {
  // check if new event coming in
  if (rcEventArray.isWritten()) {
    const LBLECharacteristicWrittenInfo &written =
        rcEventArray.getLastWrittenInfo();

    // scan and compare for event serial numbers
    for (size_t i = 0; i < m_controls.size(); i++) {

      LRemoteUIControl &control = *m_controls[i];

      // retrieve the control's event info
      rcEventArray.getValue((uint8_t *)&control.m_lastEvent, 4, i * 4);
    }
  }
}
