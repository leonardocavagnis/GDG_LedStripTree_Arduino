/*
  GDG Arduino LED Strip Tree Controller
  
  This sketch uses the ArduinoBLE library to create a Bluetooth Low Energy (BLE) device
  that controls an array of LEDs through BLE characteristics for color and effect.
  
  Author: Leonardo Cavagnis
*/

#include <ArduinoBLE.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN        6
#define LED_NUM        30

#define OFF_EFFECT      0  
#define STEADY_EFFECT   1
#define BLINK_EFFECT    2
#define ALTBLINK_EFFECT 3
#define GDG_EFFECT      4

BLEService            ledService                  ("1e03ce00-b8bc-4152-85e2-f096236d2833");
BLECharacteristic     ledColorCharacteristic      ("1e03ce01-b8bc-4152-85e2-f096236d2833", BLERead | BLEWrite, 3);
BLEByteCharacteristic ledEffectCharacteristic     ("1e03ce02-b8bc-4152-85e2-f096236d2833", BLERead | BLEWrite);

byte ledColor[3] = {255, 0, 0};
byte ledEffect = STEADY_EFFECT;

Adafruit_NeoPixel     ledStrip(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);

void applyEffect(byte effect);

void setup() {
  Serial.begin(9600);
  while (!Serial) ;

  // BLE initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE module failed!");
    while (1);
  }

  // set advertised local name and service UUID
  BLE.setLocalName("GDG-Arduino");
  BLE.setAdvertisedService(ledService);

  // add the characteristics to the CradleSmartLight service
  ledService.addCharacteristic(ledColorCharacteristic);
  ledService.addCharacteristic(ledEffectCharacteristic);
  
  // add ledService service
  BLE.addService(ledService);
  
  // set the initial value for the characteristics
  ledColorCharacteristic.writeValue(ledColor, 3);
  ledEffectCharacteristic.writeValue(ledEffect);

  // start advertising
  BLE.advertise();

  // init LED strip
  ledStrip.begin();
  ledStrip.clear();
  ledStrip.setBrightness(255);
  ledStrip.show();
}

void loop() {
  // listen for BLE peripherals to connect
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      // Check ledColor characteristic write
      if (ledColorCharacteristic.written()) {
        if (ledColorCharacteristic.valueLength() == 3) {
          ledColorCharacteristic.readValue(ledColor, 3);

          Serial.print("Color: ");
          Serial.print(ledColor[0], HEX);
          Serial.print(", ");
          Serial.print(ledColor[1], HEX);
          Serial.print(", ");
          Serial.println(ledColor[2], HEX);
        }
      }

      // Check ledEffect characteristic write
      if (ledEffectCharacteristic.written()) {
        ledEffect = ledEffectCharacteristic.value();

        Serial.print("Effect: ");
        Serial.println(ledEffect, HEX);
      }
      
      applyEffect(ledEffect);
    }
  }

  applyEffect(ledEffect);
}


unsigned long previousMillis = 0;
const long interval = 500;

void applyEffect(byte effect) {
  static bool isEvenOn = true;
  static int currentColor = 0;
  static unsigned long colorChangeMillis = 0;
  unsigned long currentMillis = millis();

  switch(effect) {
    case OFF_EFFECT:
      ledStrip.clear();
      ledStrip.show();
      break;

    case STEADY_EFFECT:
      // Set all pixels to the specified color
      for (int i = 0; i < LED_NUM; i++) {
        ledStrip.setPixelColor(i, ledColor[0], ledColor[1], ledColor[2]);
      }
      ledStrip.show();
      break;

    case BLINK_EFFECT:
      // Blink the entire strip
      if (currentMillis - previousMillis >= interval) {
        if (ledStrip.getPixelColor(0) == 0) {
          for (int i = 0; i < LED_NUM; i++) {
            ledStrip.setPixelColor(i, ledColor[0], ledColor[1], ledColor[2]);
          }
        } else {
          ledStrip.clear();
        }
        ledStrip.show();
        previousMillis = currentMillis;
      }
      break;

    case ALTBLINK_EFFECT:
      // Alternate blink effect
      if (currentMillis - previousMillis >= interval) {
        for (int i = 0; i < LED_NUM; i++) {
          if ((i % 2 == 0 && isEvenOn) || (i % 2 != 0 && !isEvenOn)) {
            ledStrip.setPixelColor(i, ledColor[0], ledColor[1], ledColor[2]);
          } else {
            ledStrip.setPixelColor(i, 0, 0, 0);
          }
        }
        ledStrip.show();
        isEvenOn = !isEvenOn;
        previousMillis = currentMillis;
      }
      break;
      
    case GDG_EFFECT:
      // GDG Effect
      if (currentMillis - colorChangeMillis >= interval) {
        for (int i = 0; i < LED_NUM; i++) {
          switch ((currentColor + i) % 4) {
            case 0:
              ledStrip.setPixelColor(i, 0x42, 0x85, 0xF4); //Blue 500
              break;
            case 1:
              ledStrip.setPixelColor(i, 0x34, 0xA8, 0x53); //Green 500
              break;
      
            case 2:
              ledStrip.setPixelColor(i, 0xF9, 0xAB, 0x00); //Yellow 600
              break;
      
            case 3:
              ledStrip.setPixelColor(i, 0xEA, 0x43, 0x35); //Red 500
              break;
            default:
              break;
          }
        }
        ledStrip.show();
      
        colorChangeMillis = currentMillis;
        currentColor = (currentColor + 1) % 4;
      }
      break;

    default:
      break;
  }
}
