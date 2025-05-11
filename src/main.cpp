#include <Arduino.h>
#include <Keyboard.h>
#include <Mouse.h>

// 定义GPIO
#define x_axis A0
#define y_axis A1
#define key_1 1
#define key_2 2
#define key_3 3
#define key_4 4

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

// 卡尔曼滤波变量
float kalman_angle = 0.0;
float kalman_P = 1.0;
const float Q_angle = 0.001;
const float R_measure = 1.0;

// 记录初始角度
float previousAngle = 0;
float totalAngle = 0;
float stepAccumulator = 0;
int rotationTurns = 0;

float kalmanUpdate(float newAngle) {
  kalman_P += Q_angle;
  float K = kalman_P / (kalman_P + R_measure);
  kalman_angle += K * (newAngle - kalman_angle);
  kalman_P *= (1 - K);
  return kalman_angle;
}

// 上一次的角度，用于计算差值
int last_angle_step = 0;
int step = 3.0;  //增量步

// 模拟按键按下和释放的函数
void pressKey(uint8_t key) {
  Keyboard.press(key); // 按下按键
  delay(pressDelay); // 保持按下一段时间
  Keyboard.release(key); // 释放按键
} 

void setup() {
  // 初始化串口通讯
  Serial.begin(250000); 
  Keyboard.begin();   //Unlike Arduino Keyboard.h, you must use begin.
  pinMode(key_1,INPUT_PULLUP);
  pinMode(key_2,INPUT_PULLUP);
  pinMode(key_3,INPUT_PULLUP);
  pinMode(key_4,INPUT_PULLUP);

  delay(500);  // 等待USB启动
}

void loop() {
  // 读取 A0 和 A1 引脚的模拟值（范围 0~1023）
  int adcValueX = analogRead(A0);
  int adcValueY = analogRead(A1);
  if(digitalRead(key_1) == LOW){
    Mouse.press(MOUSE_LEFT);
    delay(10);  // 点击持续时间
    Mouse.release(MOUSE_LEFT);
    pressKey(KEY_DELETE);
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

  float filtered_angle = kalmanUpdate(angle_deg)+180;

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


  if (stepAccumulator >= step) {
    // 右
    pressKey(KEY_RIGHT_ARROW);
    stepAccumulator = 0;
  } else if (stepAccumulator <= -step) {
    // 左
    pressKey(KEY_LEFT_ARROW);
    stepAccumulator = 0;
  }

  // 更新 A0 最大最小值
  if (adcValueX < a0_min) a0_min = adcValueX;
  if (adcValueX > a0_max) a0_max = adcValueX;
  
  // 更新 A1 最大最小值
  if (adcValueY < a1_min) a1_min = adcValueY;
  if (adcValueY > a1_max) a1_max = adcValueY;

  // 打印结果
  // Serial.print("X: ");
  // Serial.print(a0_min);
  // Serial.print(",");
  // Serial.print(a0_max);
  // Serial.print(",");
  // Serial.print(maped_X);
  // Serial.print("  |  Y: ");
  // Serial.print(a1_min);
  // Serial.print(",");
  // Serial.print(a1_max);
  // Serial.print(",");
  // Serial.print(maped_Y);
  Serial.print("step:,");
  Serial.print(stepAccumulator);
  // Serial.print("Angle:,");
  // Serial.print(angle_deg);
  // Serial.print(", Angle Increment:,");
  // Serial.print(angle_increment);
  // Serial.print(", previouse angle:,");
  // Serial.print(previousAngle);
  Serial.print(", Angle:,");
  Serial.println(totalAngle);

  // 延时 10 毫秒，避免输出过快
  delay(1);
}
    