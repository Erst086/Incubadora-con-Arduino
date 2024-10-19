#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Definiciones de los pines
#define DHTPIN 2          // Pin donde está conectado el DHT11
#define ONE_WIRE_BUS 3    // Pin donde está conectado el DS18B20
#define TZT_PWM_PIN 9     // Pin PWM para controlar el módulo TZT
#define START_BUTTON_PIN 4 // Pin donde está conectado el botón de inicio

// Tipo de sensor DHT
#define DHTTYPE DHT11     // DHT 11 

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Configuración de la pantalla LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0;
unsigned long previousHourResetMillis = 0;
int dayCounter = 0;
int hourCounter = 0;
int minuteCounter = 0;
int fourHourCounter = 0;

void setup() {
  // Inicializar la pantalla LCD
  lcd.init();
  lcd.backlight();
  
  // Inicializar el pin PWM del módulo TZT
  pinMode(TZT_PWM_PIN, OUTPUT);
  
  // Inicializar el pin del botón de inicio
  pinMode(START_BUTTON_PIN, INPUT);
}

void loop() {
  if (digitalRead(START_BUTTON_PIN) == HIGH) {
    // Inicializar el sensor DHT
    dht.begin();
    // Inicializar el sensor DS18B20
    sensors.begin();
    
    unsigned long currentMillis = millis();
    // Actualizar contador de días y horas cada minuto
    if (currentMillis - previousMillis >= 1) { // 1 minuto en milisegundos
      previousMillis = currentMillis;
      minuteCounter++;
      fourHourCounter++; // Incrementar el contador de 4 horas cada minuto
      if (minuteCounter >= 60) {
        minuteCounter = 0;
        hourCounter++;
        if (hourCounter >= 24) {
          hourCounter = 0;
          dayCounter++;
          if (dayCounter >= 26) {
            // Reiniciar el contador de días después de 26 días
            dayCounter = 0;
          }
        }
      }
    }

    // Reiniciar contador de 4 horas cada vez que llega a 4 horas
    if (fourHourCounter >= 240) { // 240 minutos en 4 horas
      fourHourCounter = 0;
      previousHourResetMillis = currentMillis;
    }

    // Leer la humedad del DHT11
    float humedad = dht.readHumidity();
    // Leer la temperatura del DS18B20
    sensors.requestTemperatures();
    float temperatura = sensors.getTempCByIndex(0);
    
    // Mostrar los datos en la pantalla LCD
    lcd.clear();
    // Mostrar contador de días y horas en la primera línea
    lcd.setCursor(0, 0);
    lcd.print("D:");
    if (dayCounter < 10) {
      lcd.print("0");
    }
    lcd.print(dayCounter);
    lcd.print(":");
    if (hourCounter < 10) {
      lcd.print("0");
    }
    lcd.print(hourCounter);
    lcd.print(":");
    if (minuteCounter < 10) {
      lcd.print("0");
    }
    lcd.print(minuteCounter);
    lcd.print("V:");
    int minutesRemaining = 240 - fourHourCounter; // Restar el tiempo transcurrido en las últimas 4 horas
    lcd.print(minutesRemaining / 60);
    lcd.print(":");
    if (minutesRemaining % 60 < 10) {
      lcd.print("0");
    }
    lcd.print(minutesRemaining % 60);
    
    // Mostrar la humedad y la temperatura en la segunda línea
    lcd.setCursor(0, 1);
    lcd.print("Hm:");
    lcd.print(humedad, 0); // Mostrar humedad sin decimales
    lcd.print("% Tmp:");
    lcd.print(temperatura, 1); // Mostrar temperatura con un decimal
    lcd.print("C");

    // Controlar la velocidad del motor basada en la temperatura
    int motorSpeed;
    if (temperatura < 37) {
      motorSpeed = 20 * 255 / 100; // 20% de la velocidad máxima
    } else if (temperatura > 39) {
      motorSpeed = 50 * 255 / 100; // 50% de la velocidad máxima
    } else {
      motorSpeed = 0; // Motor apagado
    }
    
    // Controlar el módulo TZT con la señal PWM
    analogWrite(TZT_PWM_PIN, motorSpeed);
    
  } else {
    // Si el botón no está presionado, mostrar mensaje en la pantalla LCD y apagar el motor
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Presione START");
    lcd.setCursor(0, 1);
    lcd.print("para iniciar");
    
    analogWrite(TZT_PWM_PIN, 0); // Motor apagado
  }
  
  delay(1000); // Esperar 500 ms antes de la próxima lectura
}

