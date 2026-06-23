#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>
#include <VL53L1X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  2

#define ANALOG_PIN 34
#define NUM_SAMPLES 128

int samples[NUM_SAMPLES];

MPU6050 mpu;
VL53L1X tof;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while(1);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  mpu.initialize();

  tof.setTimeout(500);
  if(tof.init()) {
    tof.startContinuous(50);
  }

  SPI.begin(18,19,23,5);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  LoRa.begin(433E6);
}

void captureWaveform() {
  for(int i=0;i<NUM_SAMPLES;i++) {
    samples[i]=analogRead(ANALOG_PIN);
    delayMicroseconds(200);
  }
}

void displayWaveform() {
  display.clearDisplay();

  for(int i=0;i<NUM_SAMPLES-1;i++) {
    int y1 = map(samples[i],0,4095,63,0);
    int y2 = map(samples[i+1],0,4095,63,0);
    display.drawLine(i,y1,i+1,y2,WHITE);
  }

  display.display();
}

void loop() {
  captureWaveform();
  displayWaveform();

  int16_t ax,ay,az,gx,gy,gz;
  mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);

  int distance = tof.read();

  LoRa.beginPacket();
  LoRa.print("AX="); LoRa.print(ax);
  LoRa.print(",AY="); LoRa.print(ay);
  LoRa.print(",AZ="); LoRa.print(az);
  LoRa.print(",DIST="); LoRa.print(distance);
  LoRa.endPacket();

  delay(500);
}
