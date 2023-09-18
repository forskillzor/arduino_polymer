#include "LiquidCrystal.h"
#include "max6675.h"   
#include <Wire.h> 

int thermoSCK = 13;                 // Указываем к какому порту подключен вывод SCK
int thermoCS = 12;                  // Указываем к какому порту подключен вывод CS
int thermoSO = 11;                  // Указываем к какому порту подключен вывод SO
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);

int SEC = 0;
int MINUT = 30;
int sec = SEC;
int min = MINUT;

int fun = 0;

int FUN_MINUT = 5;
int FUN_SEC = 0;
int fun_min = FUN_MINUT;
int fun_sec = FUN_SEC;

int timer = 0;
int heat = 0;
int HEAT_L = 2;
int FUN_L = 3;

int set_time = 0;
int set_temp = 0;

#define BTN_UP   1
#define BTN_DOWN 2
#define BTN_LEFT 3
#define BTN_RIGHT 4
#define BTN_SELECT 5
#define BTN_NONE 10

int targetTemp = 200;
int Temp = thermocouple.readCelsius();

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

float value = 49910;

void setup() {

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = value;                        // preload timer
  TCCR1B |= (1 << CS10)|(1 << CS12);    // 1024 prescaler (коэффициент деления предделителя)
  TIMSK1 |= (1 << TOIE1);               // enable timer overflow interrupt ISR (разрешаем вызов процедуры обработки прерывания переполнения счетчика)
  interrupts(); 

  pinMode(HEAT_L, OUTPUT);
  pinMode(FUN_L, OUTPUT);
  Serial.begin(9600);

  lcd.begin(16, 2);

  lcd.setCursor(0, 0);

  lcd.setCursor(0, 1);
  //pinMode(BUTTON_HEAT, INPUT_PULLUP);
  //pinMode(BUTTON_TIMER, INPUT_PULLUP);
  //attachInterrupt(0, heat_button_action, FALLING);
  //attachInterrupt(1, timer_button_action, FALLING);
  delay(500);

}

ISR(TIMER1_OVF_vect)                    // процедура обработки прерывания переполнения счетчика
{
  TCNT1 = value;                                // preload timer
  Timer();  //включаем и выключаем светодиод
  TimerFun();
}

int detectButton() {
  int keyAnalog =  analogRead(A0);
  if (keyAnalog < 100) {
    // Значение меньше 100 – нажата кнопка right
    return BTN_RIGHT;
  } else if (keyAnalog < 200) {
    // Значение больше 100 (иначе мы бы вошли в предыдущий блок результата сравнения, но меньше 200 – нажата кнопка UP
    return BTN_UP;
  } else if (keyAnalog < 400) {
    // Значение больше 200, но меньше 400 – нажата кнопка DOWN
    return BTN_DOWN;
  } else if (keyAnalog < 600) {
    // Значение больше 400, но меньше 600 – нажата кнопка LEFT
    return BTN_LEFT;
  } else if (keyAnalog < 800) {
    // Значение больше 600, но меньше 800 – нажата кнопка SELECT
    return BTN_SELECT;
  } else {
    // Все остальные значения (до 1023) будут означать, что нажатий не было
    return BTN_NONE;
  }
}


void Timer() {
  if (timer){
    sec -= 1; 
    if (sec < 0){
      sec = 59;
      min -= 1;
    }   // Чтение и отправка температуры в последовательный порт

    if (min == 0 && sec == 0) {
      
      timer = 0;
      heat = 0;
      sec = SEC;
      min = MINUT;
      heating_off();
      fun_on();
    }
  }
}

void TimerFun() {
  if (fun){
    fun_sec -= 1; 
    if (fun_sec < 0){
      fun_sec = 59;
      fun_min -= 1;
    }   // Чтение и отправка температуры в последовательный порт

    if (fun_min == 0 && fun_sec == 0) {
      
      fun = 0;
      fun_off();
      fun_sec = FUN_SEC;
      fun_min = FUN_MINUT;
    }
  }
}

void get_temp() {
  Temp = thermocouple.readCelsius();
}

void heating_on() {
  digitalWrite(HEAT_L, HIGH);
}

void heating_off() {
  digitalWrite(HEAT_L, LOW);
}

void fun_on() {
  digitalWrite(FUN_L, HIGH);
  fun = 1;
}

void fun_off() {
  digitalWrite(FUN_L, LOW);
  fun = 0;
}

void temp_control() {
  if (Temp < targetTemp) {
    heating_on();
  } else if (Temp > targetTemp+1) {
    heating_off();
  }
}

void lcd_display() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TEMP: ");
  lcd.print(Temp);
  lcd.print(" (");
  lcd.print(targetTemp);
  lcd.print(")");
  lcd.setCursor(0, 1);
  // Выводим еще немного текста, уже на другой строке
  if (fun) {
    lcd.print("BPEM: ");
    lcd.print(fun_min);
    lcd.print(":");
    lcd.print(fun_sec);
    lcd.print(" (");
    lcd.print(FUN_MINUT);
    lcd.print("m)");
  } else {
    lcd.print("BPEM: ");
    lcd.print(min);
    lcd.print(":");
    lcd.print(sec);
    lcd.print(" (");
    lcd.print(MINUT);
    lcd.print("m)");
  }
  
}
void printDisplay(const char* str) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str);
}

void temp_set() {
  delay(1000);
  while(true) {
    int button = detectButton();
    delay(100);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Temp: ");
    lcd.print(targetTemp);

    switch (button) {
    case BTN_UP:
      targetTemp += 1;
      break;
    case BTN_DOWN:
      targetTemp -= 1;
      break;
    case BTN_LEFT:
      
      break;
    case BTN_RIGHT:
     
      break;
    case BTN_SELECT:
      return;
    default:
      
      break;
    } 
  } 
}

void set_timer() {
  delay(1000);
  while(true) {
    int button = detectButton();
    delay(100);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Timer: ");
    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.print(MINUT);
    lcd.print(" minuts");

    switch (button) {
        case BTN_UP:
          MINUT += 1;
           if (MINUT > 60) MINUT = 60;
          break;
        case BTN_DOWN:
          MINUT -= 1;
            if (MINUT < 0) MINUT = 0;
          break;
        case BTN_LEFT:
          
          break;
        case BTN_RIGHT:
        
          break;
        case BTN_SELECT:
          sec = SEC;
          min = MINUT;
          return;
        default:
          
          break;
    } 
  }
}

void set_timer_fun() {
  delay(1000);
  while(true) {
    int button = detectButton();
    delay(100);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Timer FUN: ");
    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.print(FUN_MINUT);
    lcd.print(" minuts");

    switch (button) {
        case BTN_UP:
          FUN_MINUT += 1;
           if (FUN_MINUT > 60) FUN_MINUT = 60;
          break;
        case BTN_DOWN:
          FUN_MINUT -= 1;
            if (FUN_MINUT < 0) FUN_MINUT = 0;
          break;
        case BTN_LEFT:
          
          break;
        case BTN_RIGHT:
        
          break;
        case BTN_SELECT:
          fun_sec = FUN_SEC;
          fun_min = FUN_MINUT;
          return;
        default:
          
          break;
    } 
  }
}


void settings() {
  temp_set(); 
  set_timer();
  set_timer_fun();
  
  lcd.noBlink();
}

void loop() {
  int button = detectButton();

  switch (button) {
    case BTN_UP:
      fun_on();
      break;
    case BTN_DOWN:
      fun_off();
      break;
    case BTN_LEFT:
      heat = heat^1;
      delay(500);
      break;
    case BTN_RIGHT:
      timer = timer^1;
      delay(500);
      break;
    case BTN_SELECT:
      settings();
      break;
    default:
      
      break;
  }

  get_temp();
  lcd_display();

  //heat = heat^digitalRead(BUTTON_HEAT);
  //timer = timer^digitalRead(BUTTON_TIMER);

  if (heat) {
  
    temp_control();
  } else {
    heating_off();
  }

  delay(200);
}
