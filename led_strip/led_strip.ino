#include <Adafruit_NeoPixel.h>
#include <FlexiTimer2.h>
#include <EEPROM.h>

#define VERSION "1.1"
int MYADDR = 1;//芯片地址用来确定是哪个货架

#define NUMPIXELS      120    //每条灯个数
#define R Adafruit_NeoPixel::Color(35,0,0)
#define G Adafruit_NeoPixel::Color(0,35,0)
#define B Adafruit_NeoPixel::Color(0,0,35)
#define NC Adafruit_NeoPixel::Color(0,0,0)
#define PIXELS_COUNT    5     //多少条灯

enum
{
  NOP = 0,       //啥都不做
  OFF,           //灭灯
  ON_RED,        //亮红灯
  ON_GREEN,      //亮绿灯
  ON_BLUE,       //亮蓝灯
  ON_COLOR,      //亮某种颜色（还没弄）
  FLASH_RED,     //闪红灯
  FLASH_GREEN,   //闪绿灯
  FLASH_BLUE,    //闪蓝灯
};

//可用数组来保存，操作数组（当多的时候）
/**
 * #define PIN 2
 * Adafruit_NeoPixel pixelsArray[PIXELS_COUNT];
 * for(int i = 0; i < PIXELS_COUNT; i++)
 * {
 *  pixelsArray[i] = Adafruit_NeoPixel(NUMPIXELS, PIN+i, NEO_GRB + NEO_KHZ800);
 * }
*/
Adafruit_NeoPixel pixels0 = Adafruit_NeoPixel(NUMPIXELS, 2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, 3, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, 4, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels3 = Adafruit_NeoPixel(NUMPIXELS, 5, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels4 = Adafruit_NeoPixel(NUMPIXELS, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels;

//记录灯的状态：闪、亮、颜色、灭
uint8_t lightsStatus[PIXELS_COUNT][NUMPIXELS];
//闪烁变量
bool off = true;
//接收到的字符串
String str = "";
//传递给下一级的字符串
String nextStr = "";
//新的字符串
String newStr = "";
//切割的字符串数据
String dataStr[5];
int rack;       //货架
int row;        //哪条灯带（行）
int columnStart;     //列开头
int columnEnd;       //列结束
int flashDelayCount; //闪烁延时
//定时中断函数
void flash(void)
{
  flashDelayCount = 1;
}
void setup()
{

  //初始化串口
  Serial.begin(115200);
  Serial2.begin(115200);


  pinMode(22, OUTPUT); //检测状态
  //初始化灯带
  pixels1.begin();
  pixels0.begin();
  pixels2.begin();
  pixels3.begin();
  pixels4.begin();

  for (int j = 0; j < NUMPIXELS; j++)
  {
    pixels0.setPixelColor(j, R);
    pixels1.setPixelColor(j, R);
    pixels2.setPixelColor(j, R);
    pixels3.setPixelColor(j, R);
    pixels4.setPixelColor(j, R);
  }

  pixels0.show();
  pixels1.show();
  pixels2.show();
  pixels3.show();
  pixels4.show();
  delay(5000);
  for (int j = 0; j < NUMPIXELS; j++)
  {
    pixels0.setPixelColor(j, G);
    pixels1.setPixelColor(j, G);
    pixels2.setPixelColor(j, G);
    pixels3.setPixelColor(j, G);
    pixels4.setPixelColor(j, G);
  }

  pixels0.show();
  pixels1.show();
  pixels2.show();
  pixels3.show();
  pixels4.show();
  delay(5000);
  for (int j = 0; j < NUMPIXELS; j++)
  {
    pixels0.setPixelColor(j, NC);
    pixels1.setPixelColor(j, NC);
    pixels2.setPixelColor(j, NC);
    pixels3.setPixelColor(j, NC);
    pixels4.setPixelColor(j, NC);
  }

  pixels0.show();
  pixels1.show();
  pixels2.show();
  pixels3.show();
  pixels4.show();

  Serial.print("version: MakerFire_V");
  Serial.println(VERSION);

  //读取0位置的EEPROM，判断是主机还是从机
  uint8_t e2promValue = EEPROM.read(0);
  if (e2promValue == 0xff) //如果里面还没有东西，赋默认值
  {
    EEPROM.write(0, MYADDR);
  }
  else
  {
    MYADDR = e2promValue;
    if (MYADDR == 0)
    {
      Serial.println("current is master, if want to modify to slave, please input \"NO.9_mkf_0_0$\"");
    }
    else if (MYADDR == 1)
    {
      Serial.println("current is slave, if want to modify to master, please input \"NO.8_mkf_0_0$\"");
    }
  }

  //初始化定时器
  FlexiTimer2::set(100, 1.0 / 1000, flash); // call every 1 1ms "ticks"
  //启动定时器
  FlexiTimer2::start();
}

void loop()
{
  //读串口
  if (Serial.available() > 1)
  {
    //预读取一个字符
    char t_serialChar = Serial.peek();
    //收到N时开始接收
    if (t_serialChar == 'N')
    {
      //收到$符号结束
      while (t_serialChar != '$')
      {
        t_serialChar = Serial.read();
        //判断接收的字符是否是自己想要的
        if (isalnum(t_serialChar) || t_serialChar == '.' || t_serialChar == '_')
        {
          //保存到字符串中
          newStr += t_serialChar;
        }
        //字符串太长放弃接收
        if (newStr.length() > 22)
        {
          break;
        }
      }
      //分割字符串
      if (newStr.length() > 10)
      {
        int j = 0;
        str = newStr;

        while (str.indexOf('_') > 0)
        {
          for (int i = 0; i < str.indexOf('_'); i++)
          {
            if (isalnum(str.charAt(i)) || str.charAt(i) == '.')
            {
              dataStr[j] += str.charAt(i);
            }
          }
          str = str.substring(str.indexOf('_') + 1);
          j++;
        }
        for (int i = 0; i < str.length(); i++)
        {
          dataStr[j] += str.charAt(i);
        }
      }
      //数据处理到灯带状态
      dataToLights();
      //清空数据
      clearData();
    }
    else
    {
      Serial.read();
    }
  }

  //0.5S 显示一次
  if (flashDelayCount)
  {
    static uint8_t count = 0;

    flashDelayCount = 0;
    if (count > 2)
    {
      off = !off;
      digitalWrite(22, off);
      count = 0;
    }
    lightsDecide();
    pixels0.show();
    pixels1.show();
    pixels2.show();
    pixels3.show();
    pixels4.show();
    Serial2.print(nextStr + "$");
    count++;
  }
  //把串口的数据刷新
  Serial.flush();
}
void lightsDecide(void)
{
  for (int i = 0; i < PIXELS_COUNT; i++)
  {
    //选取灯带
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
    for (int j = 0; j < NUMPIXELS; j++)
    {
      //对每个灯赋予状态
      switch (lightsStatus[i][j])
      {
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
        case FLASH_RED:
          if (off)
          {
            pixels.setPixelColor(j, NC);
          }
          else
          {
            pixels.setPixelColor(j, R);
          }
          break;
        case FLASH_GREEN:
          if (off)
          {
            pixels.setPixelColor(j, NC);
          }
          else
          {
            pixels.setPixelColor(j, G);
          }
          break;
        case FLASH_BLUE:
          if (off)
          {
            pixels.setPixelColor(j, NC);
          }
          else
          {
            pixels.setPixelColor(j, B);
          }
          break;
      }
    }
  }
}
//对分割出来的字符串进行状态赋值
void dataToLights(void)
{


  if (dataStr[0].charAt(0) == 'N' &&
      dataStr[0].charAt(1) == 'O' &&
      dataStr[0].charAt(2) == '.')
  {
    String t_str;
    int i;
    for (int i = 3; i < dataStr[0].length(); i++)
    {
      if (!isdigit(dataStr[0].charAt(i)))
      {
        return;
      }
      t_str += dataStr[0].charAt(i);
    }
    rack = t_str.toInt();

    for (i = 0; i < newStr.length(); i++)
    {
      if (newStr.charAt(i) != nextStr.charAt(i))
      {
        break;
      }
    }
    if (i < newStr.length() - 1)
    {
      nextStr = newStr;
      if (rack == 8)
      {
        EEPROM.write(0, 0);
        MYADDR = 0;
        Serial.println("current is master");
      }
      else if (rack == 9)
      {
        EEPROM.write(0, 1);
        MYADDR = 1;
        Serial.println("current is slave");
      }
      else
      {
        Serial.println(nextStr);
      }
    }
    else
    {
      return;
    }
    //0号是主机
    if (MYADDR == rack)
    {

      if (dataStr[1] == "allOff")
      {
        memset(lightsStatus, OFF, PIXELS_COUNT * NUMPIXELS);
        return;
      }

      if (dataStr[4].length() == 0)
      {
        return;
      }
      uint8_t t_status = NOP;
      if (dataStr[1] == "off")
      {
        t_status = OFF;
      }
      else if (dataStr[1] == "onR")
      {
        t_status = ON_RED;
      }
      else if (dataStr[1] == "onG")
      {
        t_status = ON_GREEN;
      }
      else if (dataStr[1] == "onB")
      {
        t_status = ON_BLUE;
      }
      else if (dataStr[1] == "flashR")
      {
        t_status = FLASH_RED;
      }
      else if (dataStr[1] == "flashG")
      {
        t_status = FLASH_GREEN;
      }
      else if (dataStr[1] == "flashB")
      {
        t_status = FLASH_BLUE;
      }
      reduceStatus(t_status);
    }
  }
}

void clearData(void)
{
  for (int i = 0; i < 6; i++)
  {
    dataStr[i] = "";
  }
  str = "";
}

void reduceStatus(uint8_t t_status)
{
  if (!isdigit(dataStr[2][0]) || !isdigit(dataStr[3][0]) || !isdigit(dataStr[4][0]))
  {
    return;
  }
  row = dataStr[2].toInt();
  columnStart = dataStr[3].toInt();
  columnEnd = dataStr[4].toInt();
  if (row > PIXELS_COUNT || columnStart > columnEnd || columnEnd > NUMPIXELS - 1)
  {
    return;
  }
  for (int k = columnStart; k <= columnEnd; k++)
  {
    lightsStatus[row][k] = t_status;
  }
}

