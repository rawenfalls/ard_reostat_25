//#include <GyverBME280.h>                      // Подключение библиотеки bmp280 -датчика давления в шланге
//GyverBME280 bme; // Создание обьекта bme

#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;

#include <microDS18B20.h>
MicroDS18B20<4> ds;
MicroDS18B20<2> ds1;

#include <GyverTimers.h>
volatile byte dutyD13 = 0;

int cooler = 12;
int gas = 11;
int nagrew = 10;
int led = 9;

#include <max6675.h>
int thermoDO = 5; // Указываем к какому пину подключен SO
int thermoCS = 6; // Указываем к какому пину подключен CS
int thermoCLK = 7;// Указываем к какому пину подключен SCK
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
//#include <TroykaDHT.h>// библиотека дачика температуры внутри корпуса
//DHT dht(4, DHT11);//инициализациядатчика

//#include <NDIRZ16.h>
//#include <SoftwareSerial.h>
//Arduino UNO Pin D2 (Software Serial Rx) <===> Adaptor's Green  Wire (Tx)
//Arduino UNO Pin D3 (Software Serial Tx) <===> Adaptor's Yellow Wire (Rx)
//SoftwareSerial mySerial(2, 3);
//NDIRZ16 mySensor = NDIRZ16(&mySerial);


const byte size_arr_integr = 20; // размер массива
byte counter=0;
byte counter_speed=0;
double arr_integr[size_arr_integr] = {0};
const byte size_speed = 60; // создание массива и инициализация нулями
float arr_speed[size_speed] = {0};
double input, output, setpoint = 39;
double Kp = 0.06, Ki = 0.03, Kd = 0;//уменьшить коэффициент kp -0.015, kd -4
double integral, derivative, error, previous_error;
double sum_arr_integr =0 ;
float speed = 0;


void setup() {
  Serial.begin(9600);                         // Запуск последовательного порта
  Serial.setTimeout(5);
//  bme.begin();                                // инициализируем датчик bmp280
//  dht.begin();                                // инициализируем датчик DHT11

  pinMode(led, OUTPUT);                       //устанвливаем пины для выхода
  pinMode(cooler, OUTPUT);
  pinMode(gas, OUTPUT);
  pinMode(nagrew, OUTPUT);

  pinMode(13, OUTPUT);

  digitalWrite(led, LOW);                   //устанавливаем на пинах низкий уровень
  digitalWrite(cooler, HIGH);
  digitalWrite(gas, LOW);
  digitalWrite(nagrew, LOW);
  //mySerial.begin(9600);
  Timer1.setFrequency(50);
  Timer1.enableISR();

  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  
}
int flag_speed_first = 1;
double temperature1;
double temperature2;

int reg = 1;
int pidr = 0;
int memset_trig = 0;
void loop() {
  static uint32_t timer = millis();
  if (millis() - timer >= 1000) {
    control_signal();//управляющий сигнал
    Serial.print("Cten = "); // Отправка текста в последовательный порт
   Serial.print(thermocouple.readCelsius()); // Чтение и отправка температуры в последовательный пор
    Serial.print(" ");
  
    timer = millis();
    ds.requestTemp();
    ds1.requestTemp();
    while(!(ds.readTemp()));
      temperature1 = ds.getTemp();
      Serial.print("ds1 ");
      Serial.print(temperature2);
      Serial.print(" C");
      Serial.print(" ");
    while(!(ds1.readTemp()));
      temperature2 = ds1.getTemp();
      Serial.print("ds2_in ");
      Serial.print(temperature1);
      Serial.print(" C");
      Serial.print(" ");

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.print(" C ");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.print(" % rH ");

      //temperature1 = runMiddleArifmOptim(temperature1);
      //temperature1 = expRunningAverage(temperature1);
      time_now();//выводит время
    //gas_pressure();
    //temperature1 = temperatyre();
  
  // вычисление ошибки и обновление интеграла и производной
  error = setpoint - temperature1;
//  if(flag_speed_first == 1)
//  {
//    flag_speed_first = 0;
//    for (int counter_speed1 = 0; counter_speed1<size_speed; counter_speed1++)
//        arr_speed[counter_speed1]= temperature1;
//  }
//
//  if(counter_speed < size_speed){
//      arr_speed[counter_speed] = temperature1; counter_speed++;}
//  else {counter_speed = 0;arr_speed[counter_speed] = temperature1;}
//  speed = 0;
//
//  Serial.print("speed t/min: ");
//  if((counter_speed+1)<size_speed)
//    speed = (temperature1 - arr_speed[counter_speed+1]);
//  else
//    speed = (temperature1 - arr_speed[0]);
//  Serial.print(speed,2);
//  Serial.print(' ');
//
//  Serial.print("speed t/10sec: ");
//  if((counter_speed-10)>=0)
//    speed = (temperature1 - arr_speed[counter_speed-10]);
//  else
//    speed = (temperature1 - arr_speed[size_speed-10+counter_speed]);
//  Serial.print(speed,2);
//  Serial.print(' ');
//
//    Serial.print("speed t/30sec: ");
//  if((counter_speed-30)>=0)
//    speed = (temperature1 - arr_speed[counter_speed-30]);
//  else
//    speed = (temperature1 - arr_speed[size_speed-30+counter_speed]);
//  Serial.print(speed,2);
//  Serial.print(' ');
  
  if(error>=0.03*setpoint){
  output = Kp * error;
  integral = 0;
  derivative = 0;
  if (memset_trig) {memset(arr_integr,0,sizeof(arr_integr));
  memset_trig = 0;
  counter = 0; 
  }
  }
  else if(error<0){
  output = 0;
  integral = 0;
  derivative = 0;
  if (memset_trig) {
  memset(arr_integr,0,sizeof(arr_integr));
  memset_trig = 0;
  counter = 0; 
  sum_arr_integr = 0;
  }
  }
  else
  {
  memset_trig = 1;
    if(counter < size_arr_integr){
      arr_integr[counter] = setpoint - temperature1; counter++;}
    else {counter = 0;arr_integr[counter] = setpoint - temperature1;}
    sum_arr_integr = 0;
    for (int counter1 = 0; counter1<size_arr_integr; counter1++){
      sum_arr_integr +=arr_integr[counter1];
//      Serial.print(arr_integr[counter1]);
//      Serial.print(' ');
    }
    Serial.print("sum_arr_integr: ");
    Serial.print(sum_arr_integr);
    Serial.print(' ');
  integral = sum_arr_integr;
  
  derivative = error - previous_error;
  previous_error = error;
  // вычисление выходного значения по ПИД-алгоритму
  output = Kp * error + Ki * integral + Kd * derivative;
  }
  Serial.print("coef pide: ");
  Serial.print(Kp * error);
  Serial.print(" ");
  Serial.print(Ki * integral);
  Serial.print(" ");
  Serial.print(Kd * derivative);
  Serial.print(" ");
  Serial.print(error);


  if (pidr)
  {

    if(output >= 1.0)
      dutyD13 = 50;
    else if (output <= 0)
      dutyD13 = 0;
    else{dutyD13 = output * 1024 / 20.48;}
  }
  else
  dutyD13 = 0;
/*
  //  if (pidr == 1)
  //  {
  //    pid.input = temperature1;
  //    pid.getResultTimer();
  //    Serial.print(" ");
  //    float t = pid.output;
  //    Serial.print(t, 5);
  //    Serial.print(" ");
  //    if (t >= 1 ) { //&& pid.setpoint >= temperature1 && pid.setpoint >= (temperature1-0.2)
  //      //Serial.print("nagrew on");
  //      Serial.print("50");
  //      //digitalWrite(nagrew, 1);
  //      dutyD13 = 50;
  //    }
  //    else if (t > 0.1 && t < 1) {
  //      dutyD13 = t * 1024 / 20.48;
  //      Serial.print(" ");
  //      Serial.print(dutyD13);
  //    }
  //    else {
  //      //Serial.print("nagrew off");
  //      Serial.print("0");
  //      dutyD13 = 0;
  //      //digitalWrite(nagrew, 0);
  //    }
  //  }
  //  else
  //  {
  //    dutyD13 = 0;
  //  }
  //  Serial.println("");
  // }*/
  Serial.print(" ");
  Serial.print(dutyD13);
  Serial.print(" ");
  Serial.print(digitalRead(nagrew));
  Serial.print(digitalRead(led));
  Serial.print(digitalRead(cooler));
    Serial.println("");
}
}


const int NUM_READ = 10;  // количество усреднений для средних арифм. фильтров
// оптимальное бегущее среднее арифметическое
float runMiddleArifmOptim(float newVal) {
  static int t = 0;
  static float vals[NUM_READ];

  static float average = 0;
  if (++t >= NUM_READ) t = 0; // перемотка t
  average -= vals[t];         // вычитаем старое
  average += newVal;          // прибавляем новое
  vals[t] = newVal;           // запоминаем в массив
  return ((float)average / NUM_READ);
}

float k = 0.05;  // коэффициент фильтрации, 0.0-1.0
// бегущее среднее
float expRunningAverage(float newVal) {
  static float filVal = 0;
  filVal += (newVal - filVal) * k;
  return filVal;
}
/*
void pidControl(float temperature1) {
  //  static uint32_t tmr;
  //  if (millis() - tmr >= 1000)
  //  {
  //    tmr = millis();
  pid.input = temperature1;
  pid.getResultTimer();
  if (pidr == 1)
  {
    Serial.print(" ");
    float t = pid.output;
    Serial.print(t, 5);
    Serial.print(" ");
    if (t >= 1) {
      Serial.print("nagrew on");
      digitalWrite(nagrew, 1);
    }
    else {
      Serial.print("nagrew off");
      digitalWrite(nagrew, 0);
    }
  }
  //  }
}
*/
void time_now() {
  uint32_t sec = millis() / 1000ul;
  int timeHours = (sec / 3600ul);        // часы
  int timeMins = (sec % 3600ul) / 60ul;  // минуты
  int timeSecs = (sec % 3600ul) % 60ul;  // секунды

  Serial.print(timeHours);
  Serial.print(":");
  Serial.print(timeMins);
  Serial.print(":");
  Serial.print(timeSecs);
  Serial.print(" ");
}
/*
void co2_sensor() {
  if (mySensor.measure()) {
    Serial.print("CO2 Concentration is ");
    Serial.print(mySensor.ppm);
    Serial.print ("ppm ");
    Serial.print(mySensor.ppm / 10000.0);
    Serial.println("%");
  }
}*/
// float temperatyre() {
//  dht.read(); // считывание данных с датчика

//  switch (dht.getState()) { // проверяем состояние данных
//    case DHT_OK: // всё OK
//      Serial.print("Temperature = ");
//      temperature = dht.getTemperatureC();
//      Serial.print(temperature);
//      Serial.println(" C \t");
//      break;
//    case DHT_ERROR_CHECKSUM: // ошибка контрольной суммы
//      Serial.println("Checksum error");
//      break;
//    case DHT_ERROR_TIMEOUT:// превышение времени ожидания
//      Serial.println("Time out error");
//      break;
//    case DHT_ERROR_NO_REPLY: // данных нет, датчик не реагирует или отсутствует
//      //Serial.println("Sensor not connected");
//      Serial.print("Temperature = ");
//      Serial.print(temperature);
//      Serial.println(" C \t");
//      break;
//  }
//  return (temperature);
// }

void control_signal() { //управление происходит путем передачи в порт первой буквы устройства которым необходимо управлять,
  //управление происходит путем изменения уровня(high or low)
  if (Serial.available() > 0) {
    char key = Serial.read();//читает первый символ
    int val = Serial.parseInt();
    //int in_data = Serial.read() - '0';//принемает циру

    // int data = Serial.parseInt();// читает число
    //Serial.println(in_data);
    switch (key) {
      case 'l': if (digitalRead(led))
        {
          digitalWrite(led, LOW);
          Serial.print(" led off");
        }
        else
        {
          digitalWrite(led, HIGH);
          Serial.print(" led on");
        }
        break;
      case 'c': if (digitalRead(cooler))
        {
          digitalWrite(cooler, LOW);
          Serial.print(" cooler off");
        }
        else
        {
          digitalWrite(cooler, HIGH);
          Serial.print(" cooler on");
        }
        break;
      case 'g': if (digitalRead(gas))
        {
          digitalWrite(gas, LOW);
          Serial.print(" gas off");
        }
        else
        { digitalWrite(gas, HIGH); Serial.print(" gas on");
          delay(500);
          digitalWrite(gas, LOW); Serial.print(" gas off");
        }
        break;
      case 'n': if (digitalRead(nagrew))
        {
          digitalWrite(nagrew, LOW);
          Serial.println(" nagrew off");
        }
        else
        {
          digitalWrite(nagrew, HIGH);
          Serial.println(" nagrew on");
        }
        break;
      case 'p': if (pidr)
        {
          pidr = 0;
          Serial.print(" pid off");
          digitalWrite(nagrew, LOW);
        }
        else
        {
          pidr = 1;
          Serial.print(" pid on");
          digitalWrite(nagrew, LOW);
        }
        break;

    }
  }
}
/*
void gas_pressure() {
  //  Serial.print("Temperature: ");
  //  Serial.print(bme.readTemperature());        // Выводим темперутуру в [*C]
  //  Serial.println(" *C");

  //  Serial.print("Humidity: ");
  //  Serial.print(bme.readHumidity());           // Выводим влажность в [%]
  //  Serial.println(" %");

  float pressure = bme.readPressure();        // Читаем давление в [Па]
  Serial.print("Pressure: ");
  Serial.print(pressure / 100.0F);            // Выводим давление в [гПа]
  Serial.print(" hPa , ");
  Serial.print(pressureToMmHg(pressure));     // Выводим давление в [мм рт. столба]
  Serial.println(" mm Hg");
  //  Serial.print("Altitide: ");
  //  Serial.print(pressureToAltitude(pressure)); // Выводим высоту в [м над ур. моря]
  //  Serial.println(" m");
  //Serial.println("");
}*/

void pwmTick() {
  static volatile byte counter = 0;
  if (counter == 50) {
    counter = 0;
  }
  if (dutyD13 > 0) {
    digitalWrite(nagrew, HIGH);
  }
  if (counter >= dutyD13) {
    if (dutyD13 < 50) {
      digitalWrite(nagrew, LOW);
    }
  }
  counter++;
}

ISR(TIMER1_A) {
  //pwmTick();
}
