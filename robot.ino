#include <PS2X_lib.h>  
#include <Servo.h>
#include <Adafruit_TCS34725.h>
#include <stdio.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        13  //14    
#define PS2_CMD        11  //15
#define PS2_SEL        10  //16
#define PS2_CLK        12  //17


/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false


PS2X ps2x; // create PS2 Controller Class


//right now, the library does NOT support hot pluggable controllers, meaning
//you must always either restart your Arduino after you connect the controller,
//or call config_gamepad(pins) again after connecting the controller.


int error = 0;
byte type = 0;
byte vibrate = 0;




const int leftMotor1 = 8; // Động cơ 1 chân cắm motor ở kênh tín hiệu P8 (chiều dương)
const int leftMotor2 = 9; // Động cơ 1 chân cắm motor ở kênh tín hiệu P9 (chiều âm)
const int rightMotor1 = 10; // Động cơ 2 chân cắm motor ở kênh tín hiệu P10 (chiều dương)
const int rightMotor2 = 11; // Động cơ 2 chân cắm motor ở kênh tín hiệu P11 (chiều âm)


const int intakeMotor1  = 12 // Động cơ 3 chân cắm motor ở kênh tín hiệu P12 (chiều dương)
const int intakeMotor2 = 13; // Động cơ 3chân cắm motor ở kênh tín hiệu P13 (chiều âm)
const int shootMotor1 = 14; // Động cơ 4 chân cắm motor ở kênh tín hiệu P14 (chiều dương)
const int shootMotor2 = 15; // Động cơ 4 chân cắm motor ở kênh tín hiệu P15 (chiều âm)


//Khai báo Servo
Servo servoWrist; //Servo 360
Servo servoNOACTION; //Servo 360
Servo servoSensor; //Servo 180
Servo servoGate; //Servo 180


// Khởi tạo đối tượng PCA9685 với địa chỉ mặc định
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


//Khởi tạo cho cảm biến màu sắc tcs 34725
Adafruit_TCS34725 tcs= Adafruit_TCS34725(TCS34725_INTERGRATIONTIME_101MS,TCS34725_GAIN_4X);


//Tạo các biến
int joyY;
int joyX;
int njoyY;
int njoyX;


void setup(){
 
  Serial.begin(57600);
 
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
   


  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
 
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
  if (pressures)
    Serial.println("true ");
  else
    Serial.println("false");
  Serial.print("rumble = ");
  if (rumble)
    Serial.println("true)");
  else
    Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
  }  
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug");
   
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug");


  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
 
//  Serial.print(ps2x.Analog(1), HEX);
 
  type = ps2x.readType();
  switch(type) {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
  case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
   }
  pwm.begin(); //khởi tạo PCA9685
  pwm.setOscillatorFrequency(27000000); // cài đặt tần số dao động
  pwm.setPWMFreq(50);// cài đặt tần số PWM. Tần số PWM có thể được cài đặt trong khoảng 24-1600 HZ, tần số này được cài đặt tùy thuộc vào nhu cầu xử dụng. Để điều khiển được cả servo và động cơ DC cùng nhau, tần số PWM điều khiển được cài đặt trong khoảng 50-60Hz.
  Wire.setClock(400000); // cài đặt tốc độ giao tiếp i2c ở tốc độ cao nhất(400 Mhz). Hàm này có thể bỏ qua nếu gặp lỗi hoặc không có nhu cầu tử dụng I2c tốc độ cao
   
   // IKhởi tạo động cơ
  pinMode(leftMotor1 OUTPUT);
  pinMode(leftMotor2, OUTPUT);


  pinMode(rightMotor1, OUTPUT);
  pinMode(rightMotor2, OUTPUT);


  pinMode(intakeMotor1, OUTPUT);
  pinMode(intakeMotor2, OUTPUT);


  pinMode(shootMotor1,OUTPUT);
  pinMode(shootMotor2,OUTPUT);


  //Khởi tạo chân cắm cho servo
  servoWrist1.attach(5);
  servoWrist2.attach(4);
  servoGate.attach(2);
  servoSensor.attach(7);


  //Servo quay 1 góc 90 làm vị trí cố định
  servoSensor.write(90);


  //Lưu giá trị mặc định ban đầu của joystick
  ps2x.read_gamepad();
  joyX=ps2x.Analog(PSS_LX);
  joyY=ps2x.Analog(PSS_RY);


}




void MotorActivate(int numPWM, int val){ //Hàm điều khiển động cơ
  pwm.setPWM(numPWM, 0, val);
}


void StopWork(int numPWM){ // Hàm dừng động cơ
  pwm.setPWM(numPWM, 0, 0);
}




void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
 
  uintl6_t r, b, g, c, colorTemp, lux; //Khởi tạo các biến ứng với màu đỏ (r), xanh lam (b), xanh lá (g), trắng (c), nhiệt độ màu (colorTemp), độ rọi soi (lux)


  tcs.getRawData(&r, &g, &b, &c); // Đọc các giá trị từ cảm biến


  colorTemp = tcs.calculateColorTemperature(r, g, b); //Tính nhiệt độ màu


  lux = tcs.calculateLux(r, g, b); // TÍnh độ rọi soi
 
  if (c>r && c>b && c>g) { //Nếu giá trị màu trắng lớn hơn màu đỏ, xanh lam và xanh lá thì quay servo đưa bóng sang bên trái
    for (pos = 90; pos >= 0; pos -= 30){
        servoSensor.write(pos)
      }
      delay(1000);
  }
  else{ //Nếu không phải màu trắng thì quay servo đưa bóng sang bên phải
    for (pos = 90; pos <= 180; pos += 30){
        servoSensor.write(pos)
      }
      delay(1000);
  }


  if(error == 1) //skip loop if no controller found
    return;
 
  if(type == 2){ //Guitar Hero Controller
    ps2x.read_gamepad();  //read controller
    njoyX = ps2x.Analog(PSS_LX);
    njoyY = ps2x.Analog(PSS_RY);
    if (njoyY<joyY){ // Đi thẳng
      StopWork(8);
      StopWork(9);
      StopWork(10);
      StopWork(11);
      delay(250); //Dừng động cơ tránh việc động cơ đảo chiều đột ngột


      MotorActivate(8, 3072);
      MotorActivate(9, 0);
      MotorActivate(10, 3072);
      MotorActivate(11, 0);
    }
    else if (njoyY<joyY){ // ĐI lùi
      StopWork(8);
      StopWork(9);
      StopWork(10);
      StopWork(11);
      delay(250); //Dừng động cơ tránh việc động cơ đảo chiều đột ngột


      MotorActivate(8, 0);
      MotorActivate(9, 3072);
      MotorActivate(10, 0);
      MotorActivate(11, 3072);
    }
    else if (njoyX<joyX){ //Rẽ trái
      StopWork(8);
      StopWork(9);
      StopWork(10);
      StopWork(11);
      delay(250); //Dừng động cơ tránh việc động cơ đảo chiều đột ngột


      MotorActivate(8, 0);
      MotorActivate(9, 3072);
      MotorActivate(10, 3072);
      MotorActivate(11, 0);
    }
    else if (njoyX<joyX){ //Rẽ phải
      StopWork(8);
      StopWork(9);
      StopWork(10);
      StopWork(11);
      delay(250); //Dừng động cơ tránh việc động cơ đảo chiều đột ngột


      MotorActivate(8, 3072);
      MotorActivate(9, 0);
      MotorActivate(10, 0);
      MotorActivate(11, 3072);
    }
    else{ //Dừng lại
      StopWork(8);
      StopWork(9);
      StopWork(10);
      StopWork(11);
    }


    if (ps2x.Button(PSB_TRIANGLE)==HIGH){ //Bật motor bắn bóng
      MotorActivate(14, 3686);
      MotorActivate(15, 0);
    }
    else if (ps2x.Button(PSB_SQUARE)==HIGH){ //Tắt motor bắn bóng
      MotorActivate(14, 0);
      MotorActivate(15, 0);
    }


    if (ps2x.Button(PSB_CIRCLE)==HIGH){ //Bật motor thu thập bóng
      MotorActivate(12, 3072);
      MotorActivate(13, 0);
    }
    else if (ps2x.Button(PSB_CROSS)==HIGH){ //Tắt motor thu thập bóng
      MotorActivate(12, 0);
      MotorActivate(13, 0);
    }
    else if (ps2x.Button(PSB_R1)==HIGH){ //Giảm tốc độ motor thu thập bóng
      MotorActivate(12, 1365);
      MotorActivate(13, 0);
      delay(250); //Dừng động cơ tránh đảo chiều động cơ đột ngột


      MotorActivate(12, 1365);
      MotorActivate(13, 0);
    }
    else if (ps2x.Button(PSB_R2)==HIGH){// Đảo chiều motor thu thập bóng
      MotorActivate(12, 0);
      MotorActivate(13, 0);
      delay(250); //Dừng động cơ tránh đảo chiều động cơ đột ngột


      MotorActivate(12, 0);
      MotorActivate(13, 3072);
    }


    if (ps2x.Button(PSB_PAD_UP)==HIGH){ //Quay 2 Servo 1 góc 60 độ để nâng hộp đựng bóng đen
      for (pos = 0; pos <= 60; pos += 1){
        servoWrist.write(pos);
      }
      delay(1000);
    }
    else if (ps2x.Button(PSB_PAD_DOWN)==HIGH){ //Quay 2 Servo để đưa hộp về vị tri cũ
      for (pos = 60; pos >= 0; pos -= 1){
        servoWrist.write(pos);
      }
      delay(1000);
    }
   
    if (ps2x.Button(PSB_PAD_LEFT)==HIGH){ //Quay Servo đóng cổng không cho bóng trắng đi vào khu vực bắn
      for (pos = 90; pos >= 0; pos -= 10){
        servoGate.write(pos)
      }
      delay(1000);
    }
     else if (ps2x.Button(PSB_PAD_RIGHT)==HIGH){ //Quay Servo mở cổng cho bóng trắng đi vào khu vực bắn
      for (pos = 0; pos <= 90; pos += 10){
        servoGate.write(pos)
      }
      delay(1000);
    }


  }
  delay(50);  
}
