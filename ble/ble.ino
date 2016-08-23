#include<Wire.h>
#include <SoftwareSerial.h> 

SoftwareSerial BTSerial(2,3); 

const int MPU=0x68;  //MPU 6050 의 I2C 기본 주소
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int IRpin = 0;
int lat=37.514980,lon=127.063237; //위도와 경도
int r = 6378137; //지구 반지름
int angle=90; // 진행방향
void setup() {
  Wire.begin();      //Wire 라이브러리 초기화
  Wire.beginTransmission(MPU); //MPU로 데이터 전송 시작
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     //MPU-6050 시작 모드로
  Wire.endTransmission(true); 
  Serial.begin(9600);
  BTSerial.begin(9600); 
   
  delay(1000);

  BTSerial.write("AT");
  delay(1000);
  while (BTSerial.available()){
     Serial.write(BTSerial.read());
  }
  BTSerial.write("AT+CON74DAEAB1B7C3");
  delay(2000);
  while (BTSerial.available()){
     Serial.write(BTSerial.read());
  }
  // put your setup code here, to run once:
  
}

void loop() {
  int x,y;
  // put your main code here, to run repeatedly:
  int temp =0;
  double dis;
  BTSerial.write("AT+RSSI?");
  delay(300);
  while (BTSerial.available()){
    if(BTSerial.find("-")){
      temp = BTSerial.parseInt();
    }
  }
  BTSerial.print("RSSI : ");
  BTSerial.println((-temp));
  dis = calculateDistance(-65, temp);
  BTSerial.print("Distance : ");
  BTSerial.println(dis);
  float volts = analogRead(IRpin)*0.0048828125;   // value from sensor * (5/1024) - if running 3.3.volts then change 5 to 3.3
  float distance = 65*pow(volts, -1.10);  
  BTSerial.print("MW : ");
  BTSerial.println(distance);
  
  x = r * lon * cos(180/PI*lat);
  y = r * lat;
  BTSerial.print("x axis : ");
  BTSerial.print(x+dis*sin(angle)); //gps 진행방향 각도
  BTSerial.print(" y axis : ");
  BTSerial.println(y+dis*cos(angle));
  
  Wire.beginTransmission(MPU);    //데이터 전송시작
  Wire.write(0x3B);               // register 0x3B (ACCEL_XOUT_H), 큐에 데이터 기록
  Wire.endTransmission(false);    //연결유지
  Wire.requestFrom(MPU,14,true);  //MPU에 데이터 요청
  //데이터 한 바이트 씩 읽어서 반환
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)

  BTSerial.print(" | AcY = "); 
  BTSerial.println(AcY);
  

  if(distance<41){
    BTSerial.println("Warning Fall of Rocks" ); //초음파 낙석
  }
  if(AcY>=15500&&AcY<=16250){
    BTSerial.println("Warning Braking!!!" ); //멈췄을때
  }
    BTSerial.println();
  delay(2000);
}

double calculateDistance(int txPower, double rssi) {
  if (rssi == 0) {
    return -1.0; // if we cannot determine distance, return -1.
  }

  double ratio = rssi*1.0/txPower;
  if (ratio < 1.0) {
    return pow(ratio,10);
  }
  else {
    double accuracy =  (0.89976)*pow(ratio,7.7095) + 0.111;    
    return accuracy;
  }
}
