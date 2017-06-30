// MPU-6050 Short Example Sketch
// By Arduino User JohnChi
// August 17, 2014
// Public Domain
#include <Arduino.h>
#include <Wire.h>

int		led	= 7;
int		val	= 0;

const int	MPU_addr = 0x68;  // I2C address of the MPU-6050
int16_t		AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

uint8_t		reg_val;

void setup() {
	Serial.begin(115200);

	pinMode(led, OUTPUT);

	Wire.begin();
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x6B);	// PWR_MGMT_1 register
	Wire.write((byte)0);	// set to zero (wakes up the MPU-6050)
	Wire.endTransmission(true);

	Wire.beginTransmission(MPU_addr);
	Wire.write(0x75);
	Wire.endTransmission();

	Wire.requestFrom(MPU_addr, 1);
	reg_val = Wire.read();
	Serial.print("MPU [0x75] = 0x"); Serial.println(reg_val, HEX);

	Serial.println("Finished Setup.");
}

void loop() {
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x3B);		// starting with register 0x3B (ACCEL_XOUT_H)
	Wire.endTransmission();
	Wire.requestFrom(MPU_addr, 14);	// request a total of 14 registers

	AcX = Wire.read()<<8 | Wire.read();	// 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
	AcY = Wire.read()<<8 | Wire.read();	// 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
	AcZ = Wire.read()<<8 | Wire.read();	// 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
	Tmp = Wire.read()<<8 | Wire.read();	// 0x41 (TEMP_OUT_H)   & 0x42 (TEMP_OUT_L)
	GyX = Wire.read()<<8 | Wire.read();	// 0x43 (GYRO_XOUT_H)  & 0x44 (GYRO_XOUT_L)
	GyY = Wire.read()<<8 | Wire.read();	// 0x45 (GYRO_YOUT_H)  & 0x46 (GYRO_YOUT_L)
	GyZ = Wire.read()<<8 | Wire.read();	// 0x47 (GYRO_ZOUT_H)  & 0x48 (GYRO_ZOUT_L)

	Serial.print("AcX = ");    Serial.print(AcX, HEX);
	Serial.print(" | AcY = "); Serial.print(AcY, HEX);
	Serial.print(" | AcZ = "); Serial.print(AcZ, HEX);
	Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);
	Serial.print(" | GyX = "); Serial.print(GyX, HEX);
	Serial.print(" | GyY = "); Serial.print(GyY, HEX);
	Serial.print(" | GyZ = "); Serial.println(GyZ,HEX);

	digitalWrite(led, val&0x1);
	val++;
	delay(1000);
}
