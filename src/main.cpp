#include <Arduino.h>
#include <Keyboard.h>
#include <Mouse.h>


// Debug
//#define DEBUG

// 定义GPIO
#define x_axis A0
#define y_axis A1
#define key_1 4
#define key_2 5
#define key_3 6
#define key_4 7

// 操作模式定义
enum class OperationMode {
  KEYBOARD_MODE,   // 键盘模式
  MOUSE_MODE       // 鼠标模式
};

OperationMode currentMode = OperationMode::KEYBOARD_MODE;

// 定义最大最小值变量
int a0_min = 1023;
int a0_max = 0;
int a1_min = 1023;
int a1_max = 0;

float maped_X;
float maped_Y;

// 定义按键按下和释放的延迟时间（毫秒）
const int pressDelay = 2;
const int intervalDelay = 1000;
const int key1Delay = 50;
const int key2Delay = 200;
const int key3Delay = 200;
const int key4Delay = 100;


// 记录初始角度
float previousAngle = 0;
float totalAngle = 0;
float stepAccumulator = 0;
int rotationTurns = 0;
const float angleDeadzone = 2.0; // 角度死区，减少微小波动

// 上一次的角度，用于计算差值
int last_angle_step = 0;
int step = 3.0;  //增量步

//鼠标滚轮速度：
int mouseWheelSpeed =1;

// 上次按键状态和防抖
int lastKey1State = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// 模拟按键按下和释放的函数
void pressKey(uint8_t key, uint8_t press_delay) {
  Keyboard.press(key); // 按下按键
  delay(press_delay); // 保持按下一段时间
  Keyboard.release(key); // 释放按键
} 

// 切换操作模式
void toggleMode() {
  switch(currentMode) {
    case OperationMode::KEYBOARD_MODE:
      currentMode = OperationMode::MOUSE_MODE;
      Serial.println("Switched to MOUSE MODE");
      break;
      
    case OperationMode::MOUSE_MODE:
      currentMode = OperationMode::KEYBOARD_MODE;
      Serial.println("Switched to KEYBOARD MODE");
      break;
  }
}

// 处理按键输入
void handleKeys() {
  int currentKey1State = digitalRead(key_4);
  
  // 按键防抖处理
  if (currentKey1State != lastKey1State) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 模式切换按键（key_1）
    if (currentKey1State == LOW) {
      toggleMode();
      // 等待按键释放，避免多次触发
      while(digitalRead(key_4) == LOW);
    }
    
    // 根据当前模式处理其他按键
    switch(currentMode) {
      case OperationMode::KEYBOARD_MODE:
        // 按键4：左键点击 + 删除
        if(digitalRead(key_3) == LOW){
          Mouse.press(MOUSE_LEFT);
          delay(10);
          Mouse.release(MOUSE_LEFT);
          pressKey(KEY_DELETE, key4Delay);
        }
        
        // 按键2：下箭头
        if(digitalRead(key_1) == LOW){
          pressKey(KEY_DOWN_ARROW, key2Delay);
        }
        
        // 按键3：上箭头
        if(digitalRead(key_2) == LOW){
          pressKey(KEY_UP_ARROW, key3Delay);
        }
        break;
        
      case OperationMode::MOUSE_MODE:
        // 按键4：鼠标左键
        if(digitalRead(key_3) == LOW){
          Mouse.press(MOUSE_LEFT);
          delay(10);
          Mouse.release(MOUSE_LEFT);
        }
        
        // Chrom：返回
        if(digitalRead(key_2) == LOW){
          Keyboard.press(KEY_LEFT_ALT);
          Keyboard.press(KEY_LEFT_ARROW);
          delay(10);
          Keyboard.releaseAll();
        }
        
        // Chrom：向前
        if(digitalRead(key_1) == LOW){
          Keyboard.press(KEY_LEFT_ALT);
          Keyboard.press(KEY_RIGHT_ARROW);
          delay(10);
          Keyboard.releaseAll();
        }
        break;
    }
  }

  lastKey1State = currentKey1State;
}

void handleRotation(){
  // 读取 A0 和 A1 引脚的模拟值（范围 0~1023）
  int adcValueX = analogRead(A0);
  int adcValueY = analogRead(A1);
  if(digitalRead(key_3) == LOW){
    Mouse.press(MOUSE_LEFT);
    delay(10);  // 点击持续时间
    Mouse.release(MOUSE_LEFT);
    pressKey(KEY_DELETE,100);
  }
  if(digitalRead(key_1) == LOW){
    pressKey(KEY_DOWN_ARROW,200);
  }
  if(digitalRead(key_2) == LOW){
    pressKey(KEY_UP_ARROW,200);
  }
  if(digitalRead(key_1) == LOW){
    
  }

  adcValueX = constrain(adcValueX, 130, 920);
  adcValueY = constrain(adcValueY, 12, 1011);

  maped_X = map(adcValueX,130,920,0,1023)-511;
  maped_Y = map(adcValueY,12,1011,0,1023)-511;

  float angle_rad = atan2(maped_Y, maped_X);  // 正确顺序是 atan2(y, x)
  float angle_deg = angle_rad * 180.0 / PI;

  // int delta = abs(angle_deg - last_valid_angle);
  // 如果突变幅度太大，说明这次是异常值
  // if (delta > 30) {  // 你可以根据实际情况调整阈值，比如 20°
  //   angle_deg = last_valid_angle;  // 保持上一次的值
  // } else {
  //   last_valid_angle = angle_deg;
  // }

  float angle_increment = angle_deg - previousAngle;

  // 正确处理跨 0 度：差值范围限制在 -180 到 +180
  if (angle_increment > 180)
  {
    angle_increment -= 360; 
  }  else if (angle_increment < -180)
  {
    angle_increment += 360;
  } 
  
  // 累积模拟角度
  totalAngle += angle_increment;
  stepAccumulator += angle_increment;
  previousAngle = angle_deg;

  switch(currentMode){
    case OperationMode::KEYBOARD_MODE:
      if (stepAccumulator >= step) {
        // 右
        pressKey(KEY_RIGHT_ARROW,pressDelay);
        stepAccumulator = 0;
      } else if (stepAccumulator <= -step) {
        // 左
        pressKey(KEY_LEFT_ARROW,pressDelay);
        stepAccumulator = 0;
      }
      break;
    case OperationMode::MOUSE_MODE:
      if (stepAccumulator >= step) {
        Mouse.move(0, 0, -mouseWheelSpeed); // 下滚
        stepAccumulator = 0;
      } else if (stepAccumulator <= -step) {
        Mouse.move(0, 0, mouseWheelSpeed);  // 上滚
        stepAccumulator = 0;
      }
      break;
  }
}

void setup() {
  
  #ifdef DEBUG
    // 初始化串口通讯
    Serial.begin(250000); 
  #endif

  Keyboard.begin();   //Unlike Arduino Keyboard.h, you must use begin.
  Mouse.begin();

  pinMode(key_1,INPUT_PULLUP);
  pinMode(key_2,INPUT_PULLUP);
  pinMode(key_3,INPUT_PULLUP);
  pinMode(key_4,INPUT_PULLUP);

  delay(500);  // 等待USB启动
}

void loop() {
  
    // 处理按键输入
  handleKeys();

  handleRotation();


  // // 更新 A0 最大最小值
  // if (adcValueX < a0_min) a0_min = adcValueX;
  // if (adcValueX > a0_max) a0_max = adcValueX;
  
  // // 更新 A1 最大最小值
  // if (adcValueY < a1_min) a1_min = adcValueY;
  // if (adcValueY > a1_max) a1_max = adcValueY;
#ifdef DEBUG
  // 打印结果
  Serial.print("X: ");
  Serial.print(a0_min);
  Serial.print(",");
  Serial.print(a0_max);
  Serial.print(",");
  Serial.print(maped_X);
  Serial.print("  |  Y: ");
  Serial.print(a1_min);
  Serial.print(",");
  Serial.print(a1_max);
  Serial.print(",");
  Serial.print(maped_Y);
  Serial.print("step:,");
  Serial.print(stepAccumulator);
  // Serial.print("Angle:,");
  // Serial.print(angle_deg);
  // Serial.print(", Angle Increment:,");
  // Serial.print(angle_increment);
  Serial.print(", previouse angle:,");
  Serial.print(previousAngle);
  Serial.print(", Angle:,");
  Serial.println(totalAngle);


#endif

  // 延时 1 毫秒，避免输出过快
  delay(1);
}
    