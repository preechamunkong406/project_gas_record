#include <Wiegand.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoOTA.h>
#include <ESP32Ping.h>
#include <RTClib.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ModbusRtu.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <TridentTD_LineNotify.h>
#include <EEPROM.h>


TaskHandle_t Task0, Task1;
#define sw_door_pin   15
#define relay_pin     32
#define CS            5
#define EEPROM_SIZE   255
WIEGAND wg;
Modbus slave(1, Serial2, 0);
RTC_DS3231 ds3231;
WiFiClientSecure client;
WidgetRTC ntp;
WidgetLED led_lock(V9);
WidgetLED led_gs_log(V33);



const char* ssid = "HUAWEI_E979";
const char* password = "45TL5GHN60G";
char server[] = "blynk2.iot-cm.com";
char auth[] =   "Ma8nnWBDnufC6-jiGtCuWQI-4dt07pqU";
char LINE_TOKEN[] = "MvhNUOpZOpFBL2uWt8UfMuaJdDPNM9h2pHdeEJdkpG5";
String GOOGLE_SCRIPT_ID = "AKfycbzxHC8Xd4qjkGhzKchbSK1DH77oQpaXwdatuauLmKdh5RD5XquEKKQPK7IApLPMH_cU";
const char* host = "script.google.com";
const int httpsPort = 443;
uint16_t modbus_rtu[19];
unsigned long time_relay, time_check_internet = 0, time_couter = 0, time_sent_blynk, time_run;
unsigned long number = 0, number_last = 0, i = 0, time_unlock = 5000, time_line_use = 300, time_line_send = 600;
bool status_internet = false, status_gs_send_log = false, status_read_file_id = true, status_read_file_id_ok = false;
String string_date, string_date2, string_time;
String string_ntp_wday_thai = "";
int file_id = 0, blynk_id = 0, gs_id_blynk = 0, gs_id_max_blynk = 0;
unsigned long long_wg_rfid_code, long_sd_rfid_code[101];
unsigned int int_sd_rfid_send[101];
String string_sd_rfid_name[101], string_sd_rfid_code[101], string_sd_rfid_send[101], string_sd_gs_all[501];
unsigned int gs_id = 1, gs_id_max = 1, gs_id_max_send = 0, gs_id_send = 1;
String string_sd_gs_buffer_day, string_sd_gs_buffer_time, string_sd_gs_buffer_rfid, string_sd_gs_buffer_name, string_sd_gs_buffer_car;
String string_sd_gs_buffer_mileage, string_sd_gs_buffer_quantity_total, string_sd_gs_buffer_quantity_empty, string_sd_gs_buffer_quantity_send, string_sd_gs_buffer_quantity_back;
unsigned long mileage = 0, number_car = 0, couter_open_door = 0, couter_gs_send = 0, couter_line_quantity = 0;
bool status_update_ntp = false, status_update_ntp_ok = false, status_led_lock = false, status_led_lock_last = true, status_send_line = false, status_send_line_ok = true, status_step_line = false;
char daysOfTheWeek[7][12] = {"อา.", "จ.", "อ.", "พ.", "พฤ.", "ศ.", "ส."};
int int_date, int_day, int_day_last, int_month, int_year, int_hour, int_minute, int_second, int_second_last;
int int_set_day, int_set_month, int_set_year, int_set_hour, int_set_minute, int_set_second = 0;
String string_rct_time, string_rct_date, string_rct_date2, string_rct_year, string_rct_month, string_rct_day, string_rct_hour, string_rct_minute, string_rct_second;
bool status_sw_door = false, status_sw_door_last = true, status_spreadsheet = false, status_spreadsheet_ok = false, status_sw_gs_log = false;
bool status_sw_door_use = false, status_sw_door_send = false, status_hmi_restart = false, status_hmi_touch = false, status_sw_send_line = false, level_line_notify = false;
String string_value1, string_value2, string_value3, string_value4, string_value5, string_value6, string_value7, string_value8, string_value9, string_value10;
String string_blynk_value1, string_blynk_value2, string_blynk_value3, string_blynk_value4, string_blynk_value5, string_blynk_value6, string_blynk_value7, string_blynk_value8, string_blynk_value9, string_blynk_value10;
String string_ntp_time, string_ntp_date, string_ntp_date2, string_file_id, buffer_line_quantity_message, string_buffer_line_timer;
unsigned int int_quantity_total = 0, int_quantity_total_save = 0, int_quantity_empty = 0, int_quantity_empty_save = 0, int_quantity_send = 0, int_quantity_back = 0;
bool status_blynk_use = false, status_blynk_send = false, status_read_file_google_sheet = false, status_blynk_gs_log = true, status_clear_hmi  = false, status_screen2 = false;
String string_quantity_total, string_quantity_empty, string_quantity_send, string_quantity_back, buffer_line_message, string_time_line_use, string_time_line_send, string_int_timer[6];
unsigned int int_screen_jump = 0, index_timer_minute, index_timer_minute_buffer, index_timer = 1, int_timer_hour[6], int_timer_minute[6], index_t = 0;
bool status_line_timer[6], status_line_quantity = false, status_line_quantity_ok = false;









void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  pinMode(sw_door_pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relay_pin, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(relay_pin, HIGH);
  EEPROM.begin(EEPROM_SIZE);
  blynk_eeprom_read();
  slave.start();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println("Connecting...");
  Serial.println("ssid: " + String(ssid));
  Serial.println("password: " + String(password));

  int c = 0;
  while ( c < 40 ) {
    c++;
    delay(250);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.print("*");
    if (WiFi.status() == WL_CONNECTED) break;
  }

  ArduinoOTA.setPort(3232);
  ArduinoOTA.setHostname("OTA GAS RECORD");
  ArduinoOTA.setPassword((const char *)"1234");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else
      type = "filesystem";
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Blynk.config(auth, server, 8080);
  LINE.setToken(LINE_TOKEN);
  
  

  xTaskCreatePinnedToCore(
    Task0code,   /* Task function. */
    "Task0",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task0,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */

  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    2,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    1);          /* pin task to core 1 */
}


void loop() {
  while (true) {}
}
