# JogWheel
 DIY jog wheel for video editing （DaVinci Resolve）

 # ESP32-S3 Hall Sensor Jog Wheel

This project uses an **AS5600 magnetic hall rotary encoder** with an **ESP32-S3** board to create a DIY **jog wheel** that acts as a **USB keyboard**. As the wheel is rotated, the device sends next or last frame in video editing (DaVinci Resolve), frame by frame timeline scrubbing.

---

## ✨ Features

-  Reads angle from AS5600 via I2C
-  Detects rotation direction (CW / CCW)
-  Frame by frame jog wheel navigation control
-  ESP32-S3 HID support, fully plug-and-play on Windows

---

## 🧰 Hardware Required

| Component            | Example                    |
|---------------------|----------------------------|
| Microcontroller      | ESP32-S3 dev board  |
| Magnetic Encoder     | AS5600 (I2C interface)     |
| Magnet               | 5*3mm disc magnet |
| Jog wheel knob  | 3D Pringting          |

---

## 🔌 Wiring

| AS5600 Pin | ESP32-S3 Pin |
|------------|--------------|
| VCC        | 3.3V     |
| GND        | GND           |
| SDA        | GPIO 8        |
| SCL        | GPIO 9        |

Modify the `SDA_PIN` and `SCL_PIN` in code to match your setup.






