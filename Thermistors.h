#include <DHTesp.h> //senzor teploty DHT22
#include <OneWire.h> //wire senzoru DS18B20
#include <DallasTemperature.h> //senzor teploty DS18B20

class Thermistors {

private:
  const byte pinD1 = D1;
  DHTesp dht;
  DallasTemperature *dsb;
  int thermistor;
  float temperatureLast;
  int humidityLast;
  unsigned long lastRead; 
  OneWire wire; //gpio

  void qualityCalculation() {
    if (temperature > 30 or humidity > 80) {
      quality = 3;
    } else if (temperature > 27 or humidity > 60) {
      quality = 2;
    } else {
      quality = 1;
    } 
  }

public:
  float fixTemperature;
  int fixHumidity;
  float temperature;
  int humidity;
  String ProductName;
  int quality; //quality of air
  bool connected;
  int interval; //read interval in seconds

  Thermistors(int timeInterval = 1) {
    interval = timeInterval;
  }

  void begin(int sensorDigital) {
    pinMode(pinD1, INPUT);
    wire.begin(pinD1);
    thermistor = sensorDigital;
    connected = false;
    if (thermistor == 7) {
      dht.setup(pinD1, DHTesp::DHT11);
      ProductName = "DHT11";
      connected = true;
    } else if (thermistor == 8) {
      dht.setup(pinD1, DHTesp::DHT22);
      ProductName = "DHT22";
      connected = true;
    } else if (thermistor == 9) {
      wire.begin(pinD1);
      dsb = new DallasTemperature(&wire);
      dsb->begin();
      ProductName = "DS18B20";
      connected = true;
    }
  }

  bool loop() {
    if (connected == false) return false;
    if (millis() - lastRead >= interval*1000UL or lastRead == 0) {
      lastRead = millis();
      if (getTemperature() == true or getHumidity() == true) {
        qualityCalculation();
        return true;
      }
    }    
    return false;
  }

  bool getTemperature() {
    if (connected == false) {return false;}
    float tem;
    if (thermistor == 7 or thermistor == 8) {
      tem = dht.getTemperature();  
    } else if (thermistor == 9) {
      dsb->requestTemperatures();
      tem = dsb->getTempCByIndex(0);
    }   
    if (tem > -99 and tem < 99) {
      temperature = (round(tem * 10)/10) - fixTemperature;
      if (temperature != temperatureLast) {
        temperatureLast = temperature;
        return true;  
      }  
    }
    return false;
  }

  bool getHumidity() {
    if ((thermistor == 7 or thermistor == 8) and connected == true) {
      int humid;
      humid = round(dht.getHumidity());  
      if (humid > 0 and humid < 100) {
        humidity = humid - fixHumidity;
        if (humidity != humidityLast) {
          humidityLast = humidity;
          return true;
        }
      }
    }
    return false;
  }

};
