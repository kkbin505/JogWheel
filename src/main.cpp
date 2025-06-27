#include <Arduino.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include <AS5600.h>

#define SDA_PIN 8
#define SCL_PIN 9
#define DEBUG

// 方向判断相关变量
int last_raw = 1;           // 上一次角度（0 ~ 4095）
int angle_accumulator = 0;   // 累计角度差（正负）
const int STEP = 60;        // 对应大约 10°

AS5600 as5600;

USBHIDKeyboard Keyboard;

void setup() {
  Serial.begin(115200);
  USB.begin();          // 启动 USB 子系统
  Keyboard.begin();     // 启动 HID 键盘
  delay(1000);
 
  Wire.begin(SDA_PIN, SCL_PIN);  // 初始化 I2C
  as5600.begin(4);
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  int b = as5600.isConnected();
  Serial.print("Connect: ");
  Serial.println(b);
  delay(1000);


}

void loop() {

  int raw = as5600.rawAngle();  // 0 - 4095
  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print("  Degrees: ");
  Serial.println(as5600.rawAngle());

  if (last_raw >= 0) {
    int diff = raw - last_raw;

    // 处理过 0 回绕
    if (diff > 2048) diff -= 4096;
    if (diff < -2048) diff += 4096;

    angle_accumulator += diff;

    // 每累计约114单位 (~10度) 触发一次（4096 / 360 ≈ 11.4）
 
    while (angle_accumulator >= STEP) {
      Keyboard.write(KEY_RIGHT_ARROW);
      angle_accumulator -= STEP;
      Serial.println("→ Right");
    }

    while (angle_accumulator <= -STEP) {
        Keyboard.write(KEY_LEFT_ARROW);
        angle_accumulator += STEP;
        Serial.println("← Left");
      }
    last_raw = raw;
    delay(10);  // 推荐短延时，提高响应性
  }
    
  // Keyboard.write(KEY_RIGHT_ARROW);  // 模拟按下数字键 1
  // delay(1000);
}