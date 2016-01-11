#include <Adafruit_NeoPixel.h>
#include <FlexiTimer2.h>


#define NUMPIXELS      60
#define R Adafruit_NeoPixel::Color(255,0,0)
#define G Adafruit_NeoPixel::Color(0,255,0)
#define B Adafruit_NeoPixel::Color(0,0,255)
#define NC Adafruit_NeoPixel::Color(0,0,0)
#define PIXELS_COUNT    5

enum {
  NOP = 0,
  OFF,
  ON_RED,
  ON_GREEN,
  ON_BLUE,
  ON_COLOR,
  FLASH_RED,
  FLASH_GREEN,
  FLASH_BLUE,
};

Adafruit_NeoPixel pixels0 = Adafruit_NeoPixel(NUMPIXELS, 2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, 3, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, 4, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels3 = Adafruit_NeoPixel(NUMPIXELS, 5, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels4 = Adafruit_NeoPixel(NUMPIXELS, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels;
uint8_t lightsStatus[PIXELS_COUNT][NUMPIXELS];
bool off = true;
String str = "";
String nextStr = "";
String dataStr[5];
int rack;       //货架
int row;        //哪条灯带（行）
int columnStart;     //列开头
int columnEnd;       //列结束
int count;
//char *command[5] = {
//  "NO.0_allOff$"
//  "NO.0_status_0_0_0$",
//};
void flash(void) {
  off = !off;
  //  for (int i = 0; i < PIXELS_COUNT; i++) {
  //    pixels = pixelsArray[i];
  //    for (int j = 0; j < NUMPIXELS; j++) {
  //      if (lightsStatus[i][j] == FLASH_RED) {
  //        if (off) {
  //          pixels.setPixelColor(j, NC);
  //        } else {
  //          pixels.setPixelColor(j, R);
  //        }
  //      }
  //      if (lightsStatus[i][j] == FLASH_GREEN) {
  //        if (off) {
  //          pixels.setPixelColor(j, NC);
  //        } else {
  //          pixels.setPixelColor(j, G);
  //        }
  //      }
  //      if (lightsStatus[i][j] == FLASH_BLUE) {
  //        if (off) {
  //          pixels.setPixelColor(j, NC);
  //        } else {
  //          pixels.setPixelColor(j, B);
  //        }
  //      }
  //    }
count++;
  //  }
}
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  pixels1.begin();
  pixels0.begin();
  pixels2.begin();
  pixels3.begin();
  pixels4.begin();
  FlexiTimer2::set(128, 1.0 / 1000, flash); // call every 1 1ms "ticks"
  FlexiTimer2::start();
}

void loop() {
  if (Serial.available() > 0) {
    char t_serialChar = Serial.peek();
    if (t_serialChar == 'N') {
      while (t_serialChar != '$') {
        t_serialChar = Serial.read();
        if (isalnum(t_serialChar) || t_serialChar == '.' || t_serialChar == '_') {
          str += t_serialChar;
        }
        if (str.length() > 35) {
          break;
        }
      }
      if (str.length() < 35 || str.length() >= 10) {
        nextStr = str;
        int j = 0;
        while (str.indexOf('_') > 0) {
          for (int i = 0; i < str.indexOf('_'); i++) {
            if (isalnum(str.charAt(i)) || str.charAt(i) == '.') {
              dataStr[j] += str.charAt(i);
            }
          }
          str = str.substring(str.indexOf('_') + 1);
          j++;
        }
        for (int i = 0; i < str.length(); i++) {
          dataStr[j] += str.charAt(i);
        }
      }
      dataToLights();
      clearData();
    } else {
      Serial.read();
    }
  }
//lightsStatus[0][0] = ON_RED;
//lightsStatus[1][0] = ON_GREEN;
//lightsStatus[2][0] = ON_RED;
//lightsStatus[3][0] = ON_GREEN;
//lightsStatus[4][0] = ON_RED;

  lightsDecide();
  Serial.flush();
  if (count > 2){
  count = 0;
  pixels0.show();
  pixels1.show();
  pixels2.show();
  pixels3.show();
  pixels4.show();
  }
}
void lightsDecide(void) {
  for (int i = 0; i < PIXELS_COUNT; i++) {
    switch (i)
    {
      case 0:
      pixels = pixels0;
        break;
      case 1:
      pixels = pixels1;
        break;
      case 2:
      pixels = pixels2;
        break;
      case 3:
      pixels = pixels3;
        break;
      case 4:
      pixels = pixels4;
        break;
      default:
        break;
    }
    for (int j = 0; j < NUMPIXELS; j++) {
      switch (lightsStatus[i][j]) {
        case NOP:
          break;
        case OFF:
          pixels.setPixelColor(j, NC);
          lightsStatus[i][j] = NOP;
          break;
        case ON_RED:
          pixels.setPixelColor(j, R);
          lightsStatus[i][j] = NOP;
          break;
        case ON_GREEN:
          pixels.setPixelColor(j, G);
          lightsStatus[i][j] = NOP;
          break;
        case ON_BLUE:
          pixels.setPixelColor(j, B);
          lightsStatus[i][j] = NOP;
          break;
      }
    }
  }
}
void dataToLights(void) {
  if (dataStr[0].charAt(0) == 'N' &&
      dataStr[0].charAt(1) == 'O' &&
      dataStr[0].charAt(2) == '.') {
    String t_str;
    for (int i = 3; i < dataStr[0].length(); i++) {
      if (!isdigit(dataStr[0].charAt(i))) {
        return;
      }
      t_str += dataStr[0].charAt(i);
    }
    rack = t_str.toInt();
    if (1 == rack)
    {
      Serial.println(dataStr[1]);

      if (dataStr[1] == "allOff") {
        for (int i = 0; i < PIXELS_COUNT; i++) {
          for (int j = 0; j < NUMPIXELS; j++) {
            lightsStatus[i][j] = OFF;
          }
        }
        return;
      }
      if (dataStr[1] == "off") {
        row = dataStr[2].toInt();
        columnStart = dataStr[3].toInt();
        columnEnd = dataStr[4].toInt();
        for (int k = columnStart; k <= columnEnd; k++) {
          lightsStatus[row][k] = OFF;
        }
        return;
      }
      if (dataStr[1] == "onR") {
        row = dataStr[2].toInt();
        columnStart = dataStr[3].toInt();
        columnEnd = dataStr[4].toInt();
        for (int k = columnStart; k <= columnEnd; k++) {
          lightsStatus[row][k] = ON_RED;
        }
        return;
      }
      if (dataStr[1] == "onG") {
        row = dataStr[2].toInt();
        columnStart = dataStr[3].toInt();
        columnEnd = dataStr[4].toInt();
        for (int k = columnStart; k <= columnEnd; k++) {
          lightsStatus[row][k] = ON_GREEN;
        }
        return;
      }
      if (dataStr[1] == "onB") {
        row = dataStr[2].toInt();
        columnStart = dataStr[3].toInt();
        columnEnd = dataStr[4].toInt();
        for (int k = columnStart; k <= columnEnd; k++) {
          lightsStatus[row][k] = ON_BLUE;
        }
        return;
      }
      if (dataStr[1] == "flashG") {
        row = dataStr[2].toInt();
        columnStart = dataStr[3].toInt();
        columnEnd = dataStr[4].toInt();
        for (int k = columnStart; k <= columnEnd; k++) {
          lightsStatus[row][k] = FLASH_GREEN;
        }
        return;
      }
      if (dataStr[1] == "flashR") {
        row = dataStr[2].toInt();
        columnStart = dataStr[3].toInt();
        columnEnd = dataStr[4].toInt();
        for (int k = columnStart; k <= columnEnd; k++) {
          lightsStatus[row][k] = FLASH_RED;
        }
        return;
      }
      if (dataStr[1] == "flashB") {
        row = dataStr[2].toInt();
        columnStart = dataStr[3].toInt();
        columnEnd = dataStr[4].toInt();
        for (int k = columnStart; k <= columnEnd; k++) {
          lightsStatus[row][k] = FLASH_BLUE;
        }
        return;
      }
    }
    else
    {
      Serial1.print(nextStr + "$");
      Serial.println(nextStr);
    }
  }
}

void clearData(void) {
  for (int i = 0; i < 6; i++) {
    dataStr[i] = "";
  }
  str = "";
}


