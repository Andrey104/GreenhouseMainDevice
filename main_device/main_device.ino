#include <SoftwareSerial.h>
#include <LiquidCrystalRus.h>

#define DEBUG                 true
#define SIM800c               true
#define SIM_SERIAL            9600
#define ESP8266               true
#define ESP_SERIAL            9600
#define LCD                   true

LiquidCrystalRus lcd(12, 11, 5, 4, 3, 2);
const int buttonPin = 8;
int buttonState = 0;   
int clickCount = 0;
boolean btnPress = false;
boolean sim = false;
// ESP TX - 6 (Arduino SoftwareSerial RX)
// ESP RX - 7 (Arduino SoftwareSerial TX)
SoftwareSerial espSerial(6, 7);
SoftwareSerial simSerial(9, 10); // tx-9 rx - 10 sim800c

 
void setup()
{
    if (LCD) {
      pinMode(buttonPin, INPUT);
      lcd.begin(16, 2);
      lcdPrint("GreenHouse v0.1", 0);
    }
    if (DEBUG) {
      Serial.begin(9600);
      Serial.println("Debug mode! GH main device v 0.0.1"); 
      lcdPrint("Debug Mode", 1);
    }
    
    if (SIM800c) {
      simSerial.begin(SIM_SERIAL);
      if (DEBUG) {
        Serial.print("simSerial begin ");
        Serial.println(SIM_SERIAL);
      }
     
    }
    if (ESP8266) {
      espSerial.begin(ESP_SERIAL);
      if (DEBUG) {
        Serial.print("espSerial begin ");
        Serial.println(ESP_SERIAL);
      }
    
    }
    
    
    // gprs_init();

}
void lcdPrint(String str, int line) {
      lcd.setCursor(0, line);
      lcd.print(str);
}
void espInit() {
  
}

void clickCalc()
{
  if (btnPress) {
    
  }else{
    clickCount++;
  }
}
 
void loop() {
  serialEspDebug();
}

void serialRetranslate() {
  if ( espSerial.available() )
    Serial.write( espSerial.read() );
    if ( simSerial.available() )
    Serial.write( simSerial.read() );
    if ( Serial.available() ){
        if (!sim){
          espSerial.write( Serial.read() );
        }else{
          char readByte = Serial.read();
          if(readByte == '$'){
            simSerial.write(0x1A); // посылаем Ctrl + Z модему
          }else{
            simSerial.write( readByte );
          }
        }
    }
}
void serialEspDebug() {
    if ( espSerial.available() )
    Serial.write( espSerial.read() );
    if ( Serial.available() )
    espSerial.write( Serial.read() );
}

void buttonCheckTest() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    clickCalc();
    btnPress = true;
  } else {
    btnPress = false;
  }
   lcd.setCursor(1, 1);
   lcd.print(clickCount);
}

String readGSM() {  //функция чтения данных от GSM модуля
  int c;
  String v;
  while (simSerial.available()) {  //сохраняем входную строку в переменную v
    c = simSerial.read();
    v += char(c);
  }
  return v;
}

void gprs_init() {  //Процедура начальной инициализации GSM модуля
  int d = 500;
  int ATsCount = 7;
  String ATs[] = {  //массив АТ команд
    "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"",  //Установка настроек подключения
    "AT+SAPBR=3,1,\"APN\",\"internet.mts.ru\"",
    "AT+SAPBR=3,1,\"USER\",\"mts\"",
    "AT+SAPBR=3,1,\"PWD\",\"mts\"",
    "AT+SAPBR=1,1",  //Устанавливаем GPRS соединение
    "AT+HTTPINIT",  //Инициализация http сервиса
    "AT+HTTPPARA=\"CID\",1"  //Установка CID параметра для http сессии
  };
  int ATsDelays[] = {6, 1, 1, 1, 3, 3, 1}; //массив задержек
  Serial.println("GPRG init start");
  for (int i = 0; i < ATsCount; i++) {
    Serial.println(ATs[i]);  //посылаем в монитор порта
    simSerial.println(ATs[i]);  //посылаем в GSM модуль
    delay(d * ATsDelays[i]);
    Serial.println(readGSM());  //показываем ответ от GSM модуля
    delay(d);
  }
  Serial.println("GPRG init complete");
}

void gprs_send(String data) {  //Процедура отправки данных на сервер
  //отправка данных на сайт
  int d = 400;
  Serial.println("Send start");
  Serial.println("setup url");
  simSerial.println("AT+HTTPPARA=\"URL\",\"http://mysite.ru/?a=" + data + "\"");
  delay(d * 2);
  Serial.println(readGSM());
  delay(d);
  Serial.println("GET url");
  simSerial.println("AT+HTTPACTION=0");
  delay(d * 2);
  Serial.println(readGSM());
  delay(d);
  Serial.println("Send done");
}

    
