// #include <Arduino.h>
// #include <Ultrasonic.h>
// #include <SoftwareSerial.h>
// #include <GPRS_Shield_Arduino.h>

// Ultrasonic ultrasonic1(12, 13);

// void setup() {
//   Serial.begin(9600);
// }

// void loop() {
//   Serial.print("Sensor 01: ");
//   Serial.print(ultrasonic1.read()); // Prints the distance on the default unit (centimeters)
//   Serial.println("cm");
// }



/*=====Headers and Libraries=====*/
#include <Arduino.h>
#include <Ultrasonic.h>
#include <SoftwareSerial.h>
#include <GPRS_Shield_Arduino.h>
#include <ArduinoJson.h>

/*=====Macros=====*/
#define ID "238939abacdfe"  //Device ID
#define ECHO 11
#define TRIG 12
#define BAUDRATE 9600
#define RXPin 7
#define TXPin 8

/*=====Globals=====*/
StaticJsonDocument<512> doc;
SoftwareSerial gprs_uart(RXPin, TXPin);
//GPRS gprs(TXPin, RXPin, BAUDRATE);
Ultrasonic us1(TRIG, ECHO);

String HTTP_HOST = "http://cleanurge.herokuapp.com/";
int PORT = 80;
int bin_height = 200;
int waste_threshold = 90; //90%
String coordinates = "22.99139°N,88.4482395°E";
String sendtoserver;

//timing variables
unsigned long last_time = 0; //used to tick seconds, uses AVR Timer0
uint8_t seconds = 0;//counts 0 to 59
uint8_t minutes = 0;//counts 0 to 59
uint8_t hours = 0;//counts 0 to 23
uint8_t days = 0;//counts 0 to 6
//variable to store HTTP request rate
uint8_t http_timing = 1; //in hr
uint8_t http_event = 0; //counter variable
uint8_t alive_timing = 30; //in min
uint8_t alive_event = 0;   //counter variable

//variable to store debug message rate
uint8_t debug_event = 0;  //counter variable
//Overflow flags
bool s_ovf;
bool m_ovf;
bool h_ovf;
bool booted;  //used to do tasks once on boot
bool w_ovf;
bool en_w_ovf;

/*=====Function Prototypes=====*/
//All the function prototypes will be declared here
void init_gprs();
void init_http();
void init_sensor();
int fetch_sensor_data();  //in cm
void send_data_http();
void send_http_alive();
//counter logics
void tick_seconds();
void tick_minutes();
void tick_hours();
void tick_days();

/*=====Main Functions=====*/
void setup() {
  booted = true;
  //Serial Monitor
  Serial.begin(BAUDRATE);
  //Setting up IOs - if any

  //Setting up objects/peripherals and connection
  init_gprs();
  init_http();
  init_sensor();
}

void loop() {
  //Put the logic for function loop()
  /*===================Alerts (if any)=====================*/
  //tick the timing functions
  tick_seconds();
  tick_minutes();
  tick_hours();

  //checking the sensor data in every loop
  int waste_level = fetch_sensor_data();

  //if overflow not enabled and waste level below threshold
  if(!en_w_ovf && (waste_level < waste_threshold))
    en_w_ovf = true;  //enable overflow
  
  //checking if there is overlow of waste or not - comparing with variable "waste_threshold"
  //only if overflow enabled
  if(en_w_ovf && (waste_level >= waste_threshold))
  {
    w_ovf = true; //trigger flag
  }
  //Schedule to send sensor stats every 1hr
  // overflow results in calling the send
  if( http_event == http_timing || ( w_ovf && en_w_ovf ))
  {
    http_event = 0;//reset counter
    send_data_http();
    w_ovf = false;  //clear interrupt flag
    en_w_ovf = false; //disable interrupt
  }
  //send alive signal every 30 minutes
  if (alive_event == alive_timing)
  {
    alive_event = 0;
    send_http_alive();
  }

  //debugging section
  if(debug_event == 1)
  {
    debug_event = 0;
    //put debug messages here
    Serial.print(F("Waste level: ")); //useing F staring literals 
    Serial.println(waste_level);
    Serial.print(F("Seconds: "));
    Serial.println(seconds);
    Serial.print(F("Minutes: "));
    Serial.println(minutes);
    Serial.print(F("Hours: "));
    Serial.println(hours);
  }
  if(booted)
    booted = false;
}


/*=====User Functions go here=====*/
void init_gprs()
{
  //TODO - add the GPRS setup logic here
}
void init_http()
{
  //TODO - add HTTP connection logic
  //Checkout https://cleanurge.herokuapp.com/docs/ for accessing the routes
}
void init_sensor()
{
  //TODO - add sensor setup logic (if any)
  //sends data after boot to show device is online
  fetch_sensor_data();
  send_data_http();
}
int fetch_sensor_data()
{
  //TODO - debug logic to fetch sensor reading
  int reading = us1.read(); //reading in centimeters
  reading = map(reading, bin_height, 10,  //minimum is taken as 10cm
                0, 100);  //gives result in percentage (integer)
  return reading;
}

void send_http_alive()
{
  //GET method (/api/beacon/ID)
  //Send the ID as param
  //receive the waste threshold and update it
}

void send_data_http()
{
  //TODO - send the location and the level of waste
  //Checkout https://cleanurge.herokuapp.com/docs/ for accessing the routes

  //PUT method (/api/beacon/ID)
  //Send level (and coordinate once on boot) in request
  //receive the OK status
}

void tick_seconds()
{
  // updates every 1000ms or 1s
  uint16_t diff = millis() - last_time;
  if(diff >= 1000){
    last_time = millis();
    seconds+= (diff/1000);
    if(seconds >= 60)
    {
      s_ovf = true;
      seconds = 0;
    }
    //put task for every sec. here
    debug_event++;
  }
}
void tick_minutes()
{
  //updates every 60s or 1min
  if(s_ovf)
  {
    s_ovf = false;
    minutes++;
    if(minutes >= 60)
    {
      m_ovf = true;
      minutes = 0;
    }
    //put task for every min. here
    alive_event++;
  }
}
void tick_hours()
{
  //Updates every 60min or 1hr
  if(m_ovf)
  {
    m_ovf = false;
    hours++;
    if(hours >= 24)
    {
      h_ovf = true;
      hours = 0;
    }
    //put tasks for every hr. here
    http_event++;
  }
}
void tick_days()
{
  //Updates every 24hr or 1day
  if(h_ovf)
  {
    h_ovf = false;
    days = (days + 1) % 7;
  }
  //put tasks for every day here
}