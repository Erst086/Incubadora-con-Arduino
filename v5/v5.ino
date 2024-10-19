#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Definiciones de los pines
#define DHTPIN 2          // Pin donde está conectado el DHT11
#define ONE_WIRE_BUS 3    // Pin donde está conectado el DS18B20
#define TZT_PWM_PIN 10     // Pin PWM para controlar el módulo TZT
#define START_BUTTON_PIN 4 // Pin donde está conectado el botón de inicio
#define RESET_BUTTON_PIN 5 // Pin donde está conectado el botón de reinicio

// Tipo de sensor DHT
#define DHTTYPE DHT11     // DHT 11 

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Configuración de la pantalla LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0;
unsigned long startMillis = 0;  // Para controlar el tiempo de inicio del motor
unsigned long previousHourResetMillis = 0;
int dayCounter = 0;
int hourCounter = 0;
int minuteCounter = 0;
int fourHourCounter = 0;
bool isStarted = false;  // Estado del sistema

void setup() {
  // Inicializar la pantalla LCD
  lcd.init();
  lcd.backlight();
  
  // Inicializar el pin PWM del módulo TZT
  pinMode(TZT_PWM_PIN, OUTPUT);
  
  // Inicializar los pines de los botones
  pinMode(START_BUTTON_PIN, INPUT);
  pinMode(RESET_BUTTON_PIN, INPUT);
}

void loop() {
  if (!isStarted) {
    if (digitalRead(START_BUTTON_PIN) == HIGH) {
      isStarted = true;
      startMillis = millis();  // Guardar el tiempo de inicio
      dht.begin();
      sensors.begin();
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Presione START");
    lcd.setCursor(0, 1);
    lcd.print("para iniciar");
    delay(1000);
    return;
  }

  if (digitalRead(RESET_BUTTON_PIN) == HIGH) {
    isStarted = false;
    dayCounter = 0;
    hourCounter = 0;
    minuteCounter = 0;
    fourHourCounter = 0;
    previousMillis = 0;
    previousHourResetMillis = 0;
    // Detener el módulo TZT poniendo el pin PWM a 0
    analogWrite(TZT_PWM_PIN, 0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sistema reiniciado");
    delay(2500);
    return;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 60000) { // 1 minuto en milisegundos
    previousMillis = currentMillis;
    minuteCounter++;
    fourHourCounter++;
    if (minuteCounter >= 60) {
      minuteCounter = 0;
      hourCounter++;
      if (hourCounter >= 24) {
        hourCounter = 0;
        dayCounter++;
        if (dayCounter >= 26) {
          dayCounter = 0;
        }
      }
    }
  }

  if (fourHourCounter >= 240) { // 240 minutos en 4 horas
    fourHourCounter = 0;
    previousHourResetMillis = currentMillis;
  }

  float humedad = dht.readHumidity();
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("D:");
  if (dayCounter < 10) lcd.print("0");
  lcd.print(dayCounter);
  lcd.print(":");
  if (hourCounter < 10) lcd.print("0");
  lcd.print(hourCounter);
  lcd.print(":");
  if (minuteCounter < 10) lcd.print("0");
  lcd.print(minuteCounter);
  lcd.print("V:");
  int minutesRemaining = 240 - fourHourCounter;
  lcd.print(minutesRemaining / 60);
  lcd.print(":");
  if (minutesRemaining % 60 < 10) lcd.print("0");
  lcd.print(minutesRemaining % 60);

  lcd.setCursor(0, 1);
  lcd.print("Hm:");
  lcd.print(humedad, 0);
  lcd.print("% Tmp:");
  lcd.print(temperatura, 1);
  lcd.print("C");

  int motorSpeed;
  if (currentMillis - startMillis < 3000) {
    motorSpeed = 260; // Valor fijo de inicio durante 2 segundos
  } else {
    if (temperatura < 37) {
      motorSpeed = 60;
    } else if (temperatura > 38) {
      motorSpeed = 65;
    } else {
      motorSpeed = 75;
    }
  }
  analogWrite(TZT_PWM_PIN, motorSpeed);

  delay(1000);
}


