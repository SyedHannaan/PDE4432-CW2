unsigned char start01[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
unsigned char front_matrix[] = { 0x00, 0x00, 0x00, 0x1c, 0x22, 0x2a, 0x1a, 0x08, 0x38, 0x00, 0x3e, 0x22, 0x22, 0x3e, 0x00, 0x00 };
unsigned char back_matrix[] = { 0x00, 0x7e, 0x12, 0x2a, 0x44, 0x00, 0x3e, 0x40, 0x40, 0x40, 0x3e, 0x00, 0x7e, 0x0c, 0x10, 0x7e };
;
unsigned char left_matrix[] = { 0x7e, 0x40, 0x40, 0x00, 0x7e, 0x52, 0x52, 0x42, 0x00, 0x7e, 0x12, 0x02, 0x00, 0x02, 0x7e, 0x02 };
unsigned char right_matrix[] = { 0x7e, 0x12, 0x6c, 0x00, 0x7e, 0x00, 0x3e, 0x42, 0x72, 0x00, 0x7e, 0x08, 0x7e, 0x02, 0x7e, 0x02 };
unsigned char STOP01[] = { 0x2E, 0x2A, 0x3A, 0x00, 0x02, 0x3E, 0x02, 0x00, 0x3E, 0x22, 0x3E, 0x00, 0x3E, 0x0A, 0x0E, 0x00 };
unsigned char clear[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#define SCL_Pin A5
#define SDA_Pin A4
#include <SR04.h>
#define TRIG_PIN 12
#define ECHO_PIN 13
SR04 sr04 = SR04(ECHO_PIN, TRIG_PIN);
long distance, distance1, distance2, distance3;
const int left_ctrl = 4;
const int left_pwm = 5;
const int right_ctrl = 2;
const int right_pwm = 6;
const int sensor_l = 11;
const int sensor_c = 7;
const int sensor_r = 8;
int l_val, c_val, r_val;
const int servopin = 10;
char BLE_val;
void setup() {
  Serial.begin(9600);
  servopulse(servopin, 90);
  pinMode(10, INPUT);  //initialize Flame sensor as input.
  pinMode(9, OUTPUT);  // initialize digital pin LED as an output.
  Serial.begin(9600);
  pinMode(left_ctrl, OUTPUT);
  pinMode(left_pwm, OUTPUT);
  pinMode(right_ctrl, OUTPUT);
  pinMode(right_pwm, OUTPUT);
  pinMode(sensor_l, INPUT);
  pinMode(sensor_c, INPUT);
  pinMode(sensor_r, INPUT);
  pinMode(SCL_Pin, OUTPUT);
  pinMode(SDA_Pin, OUTPUT);  //Clear the screen
  matrix_display(clear);
  matrix_display(start01);
}
void loop() {
  if (digitalRead(10) == 0) {
    digitalWrite(9, HIGH);  // Led ON
    Serial.println("** Fire detected!!! **");
  } else {
    digitalWrite(9, LOW);  // Led OFF
    Serial.println("No Fire detected");
  }
  delay(100);
  if (Serial.available() > 0) {
    BLE_val = Serial.read();
    Serial.println(BLE_val);
  }
  switch (BLE_val) {
    case 'F':
      front();
      matrix_display(front_matrix);
      break;
    case 'B':
      back();
      matrix_display(back_matrix);
      break;
    case 'L':
      left();
      matrix_display(left_matrix);
      break;
    case 'R':
      right();
      matrix_display(right_matrix);
      break;
    case 'S':
      Stop();
      matrix_display(STOP01);
      break;
    case 'X': tracking(); break;
    case 'Y': avoid(); break;
    case 'U': follow_car(); break;
  }
}
void avoid() {
  matrix_display(start01);
  int track_flag = 0;
  while (track_flag == 0) {
    distance1 = sr04.Distance();
    if ((distance1 < 20) && (distance1 != 0)) {
      Stop2();
      delay(100);
      servopulse(servopin, 180);
      delay(500);
      distance2 = sr04.Distance();
      delay(100);
      servopulse(servopin, 0);
      delay(500);
      distance3 = sr04.Distance();
      delay(100);
      if (distance2 > distance3) {
        left();
        servopulse(servopin, 90);
      } else {
        right();
        servopulse(servopin, 90);
      }
    } else {
      front();
    }
    if (Serial.available() > 0) {
      BLE_val = Serial.read();
      if (BLE_val == 'S') {
        track_flag = 1;
      }
    }
  }
}
void follow_car() {
  matrix_display(start01);
  servopulse(servopin, 90);
  int track_flag = 0;
  while (track_flag == 0) {
    distance = sr04.Distance();
    if (distance < 5) {
      front();
    } else if ((distance >= 5) && (distance < 10)) {
      Stop();
    } else if ((distance >= 10) && (distance < 35)) {
      back2();
    } else {
      Stop();
    }
    if (Serial.available() > 0) {
      BLE_val = Serial.read();
      if (BLE_val == 'S') {
        track_flag = 1;
      }
    }
  }
}
void servopulse(int servopin, int myangle) {
  for (int i = 0; i < 30; i++) {
    int pulsewidth = (myangle * 11) + 500;
    digitalWrite(servopin, HIGH);
    delayMicroseconds(pulsewidth);
    digitalWrite(servopin, LOW);
    delay(20 - pulsewidth / 1000);
  }
}
void tracking() {
  matrix_display(start01);
  int track_flag = 0;
  while (track_flag == 0) {
    l_val = digitalRead(sensor_l);
    c_val = digitalRead(sensor_c);
    r_val = digitalRead(sensor_r);
    if (c_val == 1) {
      front2();
    } else {
      if ((l_val == 1) && (r_val == 0)) {
        left();
      } else if ((l_val == 0) && (r_val == 1)) {
        right();
      } else {
        Stop();
      }
    }
    if (Serial.available() > 0) {
      BLE_val = Serial.read();
      if (BLE_val == 'S') {
        track_flag = 1;
      }
    }
  }
}
void front() {
  digitalWrite(left_ctrl, HIGH);
  analogWrite(left_pwm, 220);
  digitalWrite(right_ctrl, HIGH);
  analogWrite(right_pwm, 190);
}
void front2() {
  digitalWrite(left_ctrl, HIGH);
  analogWrite(left_pwm, 75);
  digitalWrite(right_ctrl, HIGH);
  analogWrite(right_pwm, 70);
}
void back() {
  digitalWrite(left_ctrl, LOW);
  analogWrite(left_pwm, 220);
  digitalWrite(right_ctrl, LOW);
  analogWrite(right_pwm, 190);
}
void back2() {
  digitalWrite(left_ctrl, LOW);
  analogWrite(left_pwm, 110);
  digitalWrite(right_ctrl, LOW);
  analogWrite(right_pwm, 90);
}
void left() {
  digitalWrite(left_ctrl, LOW);
  analogWrite(left_pwm, 220);
  digitalWrite(right_ctrl, HIGH);
  analogWrite(right_pwm, 190);
}
void right() {
  digitalWrite(left_ctrl, HIGH);
  analogWrite(left_pwm, 220);
  digitalWrite(right_ctrl, LOW);
  analogWrite(right_pwm, 190);
}
void Stop() {
  analogWrite(left_pwm, 0);
  analogWrite(right_pwm, 0);
}
void Stop2() {
  digitalWrite(left_ctrl, LOW);
  analogWrite(left_pwm, 200);
  digitalWrite(right_ctrl, LOW);
  analogWrite(right_pwm, 200);
  delay(50);
  analogWrite(left_pwm, 0);
  analogWrite(right_pwm, 0);
}
void matrix_display(unsigned char matrix_value[]) {
  IIC_start();
  IIC_send(0xc0);
  for (int i = 0; i < 16; i++) {
    IIC_send(matrix_value[i]);
  }
  IIC_end();
  IIC_start();
  IIC_send(0x8A);
  IIC_end();
}
void IIC_start() {
  digitalWrite(SCL_Pin, HIGH);
  delayMicroseconds(3);
  digitalWrite(SDA_Pin, HIGH);
  delayMicroseconds(3);
  digitalWrite(SDA_Pin, LOW);
  delayMicroseconds(3);
}
void IIC_send(unsigned char send_data) {
  for (char i = 0; i < 8; i++) {
    digitalWrite(SCL_Pin, LOW);
    delayMicroseconds(3);
    if (send_data & 0x01) {
      digitalWrite(SDA_Pin, HIGH);
    } else {
      digitalWrite(SDA_Pin, LOW);
    }
    delayMicroseconds(3);
    digitalWrite(SCL_Pin, HIGH);
    send_data = send_data >> 1;
  }
}
void IIC_end() {
  digitalWrite(SCL_Pin, LOW);
  delayMicroseconds(3);
  digitalWrite(SDA_Pin, LOW);
  delayMicroseconds(3);
  digitalWrite(SCL_Pin, HIGH);
  delayMicroseconds(3);
  digitalWrite(SDA_Pin, HIGH);
  delayMicroseconds(3);
}