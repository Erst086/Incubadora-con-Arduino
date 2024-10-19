#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Definiciones de los pines
#define DHTPIN 2        // Pin donde está conectado el DHT11
#define ONE_WIRE_BUS 3  // Pin donde está conectado el DS18B20
#define TZT_PWM_PIN 9   // Pin PWM para controlar el módulo TZT

// Tipo de sensor DHT
#define DHTTYPE DHT11   // DHT 11 

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Configuración de la pantalla LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Inicializar el sensor DHT
  dht.begin();
  // Inicializar el sensor DS18B20
  sensors.begin();
  // Inicializar la pantalla LCD
  lcd.init();
  lcd.backlight();
  // Inicializar el pin PWM del módulo TZT
  pinMode(TZT_PWM_PIN, OUTPUT);
  
  // Iniciar el motor al 100% de la velocidad máxima inicialmente
  int initialSpeed = 30;
  analogWrite(TZT_PWM_PIN, initialSpeed);
  
  delay(2000); // Esperar 2 segundos antes de comenzar a leer la temperatura
}

void loop() {
  // Leer la humedad del DHT11
  float humedad = dht.readHumidity();
  // Leer la temperatura del DS18B20
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);
  
  // Verificar si el sensor de temperatura está conectado
  if (temperatura != DEVICE_DISCONNECTED_C) {
    // Mostrar los datos en la pantalla LCD
    lcd.clear();
    // Posicionar el cursor en la segunda línea
    lcd.setCursor(0, 1);
    lcd.print("Hm:");
    lcd.print(humedad, 0); // Mostrar humedad sin decimales
    lcd.print("% Tmp:");
    lcd.print(temperatura, 1); // Mostrar temperatura con un decimal
    lcd.print("C");

    // Controlar la velocidad del motor basada en la temperatura
    int motorSpeed;
    if (temperatura < 37) {
      motorSpeed = 12; // 20% de la velocidad máxima
    } else if (temperatura > 39) {
      motorSpeed = 25; // 50% de la velocidad máxima
    } else {
      motorSpeed = 20; // Motor apagado
    }
    
    // Controlar el módulo TZT con la señal PWM
    analogWrite(TZT_PWM_PIN, motorSpeed);
  } else {
    // Si el sensor de temperatura está desconectado, apagar el motor
    analogWrite(TZT_PWM_PIN, 0);
  }
  
  delay(500); // Esperar 500 ms antes de la próxima lectura
}
