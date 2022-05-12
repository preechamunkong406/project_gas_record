void Task1code(void * pvParameters) {
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println();
  Serial.println("IP address : " + WiFi.localIP().toString());
  Serial.println("Gateway    : " + WiFi.gatewayIP().toString());
  Serial.println("Subnet Mask: " + WiFi.subnetMask().toString());
  check_internet();

  for (;;) {
    if (status_internet) {
      Blynk.run();

      if (status_sw_gs_log && gs_id_max > 1 && !status_spreadsheet_ok && !status_sw_door && long_wg_rfid_code < 1 && number_car < 1 && mileage < 1 && int_quantity_send < 1 && int_quantity_back < 1 && number >= couter_gs_send + 10) {
        status_gs_send_log = true;
        gs_id = gs_id_send;
        gs_id_blynk = gs_id_send;
        gs_id_max_send = gs_id_max - 1;
        decoder_gs_text();
        status_spreadsheet = true;
        status_blynk_gs_log = true;

        if (gs_id_send >= gs_id_max_send) {
          gs_id = 1;
          gs_id_max = 1;
          gs_id_send = 1;
          gs_id_blynk = 0;
          gs_id_max_blynk = 0;
          writeFile(SD, "/google_sheet.txt", "");
        }
      }

      if (status_send_line && !status_send_line_ok) {
        status_send_line_ok = true;
        sent_line();
      }

      if (status_line_quantity && !status_line_quantity_ok) {
        status_line_quantity_ok = true;
        couter_line_quantity = number;
        sent_line_quantity();
      }

      if (status_spreadsheet) {
        status_spreadsheet = false;
        spreadsheet_comm();
      }
    }

    if (Blynk.connected()) {
      if (int_second != int_second_last) {
        int_second_last = int_second;
        sent_blynk();
      }
      if (status_blynk_use) {
        status_blynk_use = false;
        sent_blynk();
        Blynk.virtualWrite(V18, string_blynk_value7);
        Blynk.virtualWrite(V23, string_file_id + "." + string_blynk_value4);
        Blynk.virtualWrite(V24, string_blynk_value1);
        Blynk.virtualWrite(V25, string_blynk_value2);
        Blynk.virtualWrite(V26, string_blynk_value5);
        Blynk.virtualWrite(V27, string_blynk_value6);
        Blynk.virtualWrite(V30, "  " + string_blynk_value3);
        Blynk.virtualWrite(V32, string_blynk_value8);
      }
      if (status_blynk_send) {
        status_blynk_send = false;
        sent_blynk();
        Blynk.virtualWrite(V3, string_file_id + "." + string_blynk_value4);
        Blynk.virtualWrite(V4, string_blynk_value1);
        Blynk.virtualWrite(V5, string_blynk_value2);
        Blynk.virtualWrite(V6, string_blynk_value9);
        Blynk.virtualWrite(V7, string_blynk_value10);
        Blynk.virtualWrite(V18, string_blynk_value7);
        Blynk.virtualWrite(V29, "  " + string_blynk_value3);
        Blynk.virtualWrite(V32, string_blynk_value8);
      }

      if (status_read_file_id_ok && number > 10) {
        status_read_file_id_ok = false;
        Blynk.virtualWrite(V13, "clr");
        Blynk.virtualWrite(V31, "clr");
        for (file_id = 1; file_id < 101; ++file_id) {
          Blynk.virtualWrite(V13, "add", file_id, String(file_id) + "." + string_sd_rfid_name[file_id], string_sd_rfid_code[file_id]);
          if (int_sd_rfid_send[file_id]) {
            Blynk.virtualWrite(V31, "add", file_id, String(file_id) + "." + string_sd_rfid_name[file_id], string_sd_rfid_code[file_id]);
          }
        }
      }

    } else {
      if (millis() > time_check_internet + 15000) {
        time_check_internet = millis();
        check_internet();
      }
    }

    if (millis() > time_couter + 1000) {
      time_couter = millis();
      couter();
    }
  }
}


void sent_line() {
  Serial.println();
  Serial.println(buffer_line_message);
  LINE.notify(buffer_line_message);
}

void sent_line_quantity() {
  Serial.println();
  Serial.println(buffer_line_quantity_message);
  LINE.notify(buffer_line_quantity_message);
}

void spreadsheet_comm(void) {
  HTTPClient http;
  status_spreadsheet_ok = true;
  led_gs_log.on();
  Serial.println("Send google sheet");

  //value1=วันที่  value2=เวลา  value3=รหัสRFID  value4=ชื่อพนักงาน  value5=รถคันที่  value6=เลขไมล์รถ  value7=จำนวนถังเต็ม  value8=จำนวนถังเปล่า  value9=จำนวนถังส่ง  value10=จำนวนถังกลับ
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?value1=" + string_value1 + "&value2=" + string_value2
               + "&value3=" + string_value3 + "&value4=" + string_value4 + "&value5=" + string_value5 + "&value6=" + string_value6
               + "&value7=" + string_value7 + "&value8=" + string_value8 + "&value9=" + string_value9 + "&value10=" + string_value10;
  //Serial.print(url);
  //Serial.println("Making a request");
  http.begin(url.c_str()); //Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  String payload;
  if (httpCode > 0) { //Check for the returning code
    payload = http.getString();
    Serial.println(httpCode);
    //Serial.println(payload);
    if (httpCode == 200 && status_gs_send_log) {
      status_gs_send_log = false;
      gs_id_send++;
    }
  }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end();
  led_gs_log.off();
  couter_gs_send = number;
  status_spreadsheet_ok = false;
}

void sent_blynk() {
  Blynk.virtualWrite(V2, string_rct_time);

  if (status_update_ntp) update_ntp();

  if (int_day_last != int_day) {
    int_day_last = int_day;
    Blynk.virtualWrite(V1, string_rct_date2);
  }

  if (status_sw_door != status_sw_door_last) {
    status_sw_door_last = status_sw_door;
    if (status_sw_door) {
      Blynk.virtualWrite(V8, true);
    } else {
      Blynk.virtualWrite(V8, false);
    }
  }

  if (status_led_lock != status_led_lock_last) {
    status_led_lock_last = status_led_lock;
    if (status_led_lock) {
      led_lock.on();
    } else {
      led_lock.off();
    }
  }

  if (status_blynk_gs_log) {
    status_blynk_gs_log = false;
    Blynk.virtualWrite(V34, gs_id_max_blynk);
    Blynk.virtualWrite(V35, gs_id_blynk);
  }
}

BLYNK_WRITE(V0) {
  if (param.asInt()) {
    status_hmi_restart = true;
    delay(100);
    ESP.restart();
  }
}

BLYNK_WRITE(V10) {
  if (param.asInt()) {
    action_unlock();
  }
}

BLYNK_WRITE(V11) {
  if (param.asInt()) {
    status_update_ntp = true;
    status_read_file_google_sheet = true;
  }
}

BLYNK_WRITE(V12) {
  if (param.asInt()) {
    blynk_eeprom_read();
    read_file_quantity(SD, "/quantity_gas.txt");
    status_read_file_id = true;
    Blynk.virtualWrite(V18, int_quantity_total);
    Blynk.virtualWrite(V19, int_quantity_total);
    Blynk.virtualWrite(V21, int_quantity_empty);
    Blynk.virtualWrite(V32, int_quantity_empty);
    Blynk.virtualWrite(V38, int_timer_minute[1]);
    Blynk.virtualWrite(V39, int_timer_minute[2]);
    Blynk.virtualWrite(V40, int_timer_minute[3]);

    index_timer = 4;
    long startAt4 = ((int_timer_hour[index_timer] * 60) + int_timer_minute[index_timer]) * 60;
    Blynk.virtualWrite(V41, startAt4, 0, "Asia/Bangkok");
    index_timer = 5;
    long startAt5 = ((int_timer_hour[index_timer] * 60) + int_timer_minute[index_timer]) * 60;
    Blynk.virtualWrite(V42, startAt5, 0, "Asia/Bangkok");
  }
}

BLYNK_WRITE(V14) {
  blynk_id = param.asInt();
  Blynk.virtualWrite(V16, string_sd_rfid_code[blynk_id]);
  if (string_sd_rfid_name[blynk_id] == "") {
    Blynk.virtualWrite(V17, " ");
  } else {
    Blynk.virtualWrite(V17, string_sd_rfid_name[blynk_id]);
  }
  Blynk.virtualWrite(V28, int_sd_rfid_send[blynk_id]);
}

BLYNK_WRITE(V15) {
  if (param.asInt()) {
    String string_save_file_buffer = "/rfid" + String(blynk_id) + ".txt";
    String string_save_data_buffer = string_sd_rfid_code[blynk_id] + "," + string_sd_rfid_name[blynk_id] + "," + string_sd_rfid_send[blynk_id];
    char char_save_file_buffer[15];
    char char_save_data_buffer[100];
    string_save_file_buffer.toCharArray(char_save_file_buffer, 15);
    string_save_data_buffer.toCharArray(char_save_data_buffer, 100);
    writeFile(SD, char_save_file_buffer, char_save_data_buffer);
    Serial.println(char_save_file_buffer);
    Serial.println(char_save_data_buffer);
    Serial.println();
  }
}

BLYNK_WRITE(V16) {
  string_sd_rfid_code[blynk_id] = param.asStr();
}

BLYNK_WRITE(V17) {
  string_sd_rfid_name[blynk_id] = param.asStr();
}

BLYNK_WRITE(V19) {
  int_quantity_total_save = param.asInt();
}

BLYNK_WRITE(V20) {
  if (param.asInt()) {
    int_quantity_total = int_quantity_total_save;
    write_sd_quantity();
    Blynk.virtualWrite(V18, int_quantity_total);
  }
}

BLYNK_WRITE(V21) {
  int_quantity_empty_save = param.asInt();
}

BLYNK_WRITE(V22) {
  if (param.asInt()) {
    int_quantity_empty = int_quantity_empty_save;
    write_sd_quantity();
    Blynk.virtualWrite(V32, int_quantity_empty);
  }
}

BLYNK_WRITE(V28) {
  string_sd_rfid_send[blynk_id] = param.asStr();
}

BLYNK_WRITE(V36) {
  if (param.asInt()) {
    status_sw_gs_log = true;
  } else {
    status_sw_gs_log = false;
  }
}

BLYNK_WRITE(V37) {
  if (param.asInt()) {
    status_sw_send_line = true;
    status_send_line = false;
    status_send_line_ok = true;
    status_step_line = false;
    couter_open_door = number;
  } else {
    status_sw_send_line = false;
  }
}

BLYNK_WRITE(V38) {
  index_timer = 1;
  int_timer_minute[index_timer] = param.asInt();
  Serial.println(int_timer_minute[index_timer]);
}

BLYNK_WRITE(V39) {
  index_timer = 2;
  int_timer_minute[index_timer] = param.asInt();
  Serial.println(int_timer_minute[index_timer]);
}

BLYNK_WRITE(V40) {
  index_timer = 3;
  int_timer_minute[index_timer] = param.asInt();
  Serial.println(int_timer_minute[index_timer]);
}

BLYNK_WRITE(V41) {
  index_timer = 4;
  TimeInputParam t(param);
  if (t.hasStartTime()) {
    int_timer_hour[index_timer] = t.getStartHour();
    int_timer_minute[index_timer] = t.getStartMinute();
  } else {
    int_timer_hour[index_timer] = 0;
    int_timer_minute[index_timer] = 0;
  }
  blynk_eeprom_write();
}

BLYNK_WRITE(V42) {
  index_timer = 5;
  TimeInputParam t(param);
  if (t.hasStartTime()) {
    int_timer_hour[index_timer] = t.getStartHour();
    int_timer_minute[index_timer] = t.getStartMinute();
  } else {
    int_timer_hour[index_timer] = 0;
    int_timer_minute[index_timer] = 0;
  }
  blynk_eeprom_write();
}

BLYNK_WRITE(V43) {
  if (param.asInt()) {
    index_timer = 1;
    int_timer_hour[index_timer] = 0;
    blynk_eeprom_write();
  }
}

BLYNK_WRITE(V44) {
  if (param.asInt()) {
    index_timer = 2;
    int_timer_hour[index_timer] = 0;
    blynk_eeprom_write();
  }
}

BLYNK_WRITE(V45) {
  if (param.asInt()) {
    index_timer = 3;
    int_timer_hour[index_timer] = 0;
    blynk_eeprom_write();
  }
}

BLYNK_WRITE(V46) {
  if (param.asInt()) {
    status_line_timer[4] = true;
  } else {
    status_line_timer[4] = false;
  }
}

BLYNK_WRITE(V47) {
  if (param.asInt()) {
    status_line_timer[5] = true;
  } else {
    status_line_timer[5] = false;
  }
}

void blynk_eeprom_write() {
  index_timer_minute_buffer = index_timer;
  index_timer_minute = index_timer_minute_buffer + 10;
  EEPROM.write(index_timer, int_timer_hour[index_timer]);
  EEPROM.write(index_timer_minute, int_timer_minute[index_timer]);
  EEPROM.commit();
  Serial.println("Write T" + String(index_timer) + " = " + String(int_timer_hour[index_timer]) + ":" + String(int_timer_minute[index_timer]));
  blynk_eeprom_read();
}

void blynk_eeprom_read() {
  for (index_timer = 1; index_timer < 6; index_timer++) {
    index_timer_minute_buffer = index_timer;
    index_timer_minute = index_timer_minute_buffer + 10;
    int_timer_hour[index_timer] = EEPROM.read(index_timer);
    int_timer_minute[index_timer] = EEPROM.read(index_timer_minute);
    //Serial.println("Read T" + String(index_timer) + " = " + String(int_timer_hour[index_timer]) + ":" + String(int_timer_minute[index_timer]));
  }

  unsigned long time_unlock_buffer = int_timer_minute[1];
  time_unlock = time_unlock_buffer * 1000;
  string_time_line_use = String(int_timer_minute[2]);
  unsigned int time_line_use_buffer = int_timer_minute[2];
  time_line_use = time_line_use_buffer * 60;
  string_time_line_send = String(int_timer_minute[3]);
  unsigned int time_line_send_buffer = int_timer_minute[3];
  time_line_send = time_line_send_buffer * 60;

  String string_int_timer_hour4, string_int_timer_hour5, string_int_timer_minute4, string_int_timer_minute5;
  if (int_timer_hour[4] < 10) {
    string_int_timer_hour4 = "0" + String(int_timer_hour[4]);
  } else {
    string_int_timer_hour4 = String(int_timer_hour[4]);
  }
  if (int_timer_minute[4] < 10) {
    string_int_timer_minute4 = "0" + String(int_timer_minute[4]);
  } else {
    string_int_timer_minute4 = String(int_timer_minute[4]);
  }

  if (int_timer_hour[5] < 10) {
    string_int_timer_hour5 = "0" + String(int_timer_hour[5]);
  } else {
    string_int_timer_hour5 = String(int_timer_hour[5]);
  }
  if (int_timer_minute[5] < 10) {
    string_int_timer_minute5 = "0" + String(int_timer_minute[5]);
  } else {
    string_int_timer_minute5 = String(int_timer_minute[5]);
  }
  string_int_timer[4] = string_int_timer_hour4 + ":" + string_int_timer_minute4 + ":00";
  string_int_timer[5] = string_int_timer_hour5 + ":" + string_int_timer_minute5 + ":00";

  Serial.println("time_unlock = " + String(time_unlock));
  Serial.println("time_line_use = " + String(time_line_use));
  Serial.println("time_line_send = " + String(time_line_send));
  Serial.println("string_int_timer4 = " + string_int_timer[4]);
  Serial.println("string_int_timer5 = " + string_int_timer[5]);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V36);
  Blynk.syncVirtual(V37);
  Blynk.syncVirtual(V46);
  Blynk.syncVirtual(V47);
  led_lock.off();
  ntp.begin();
  status_update_ntp = true;
  status_read_file_id = true;
  status_read_file_google_sheet = true;
  read_file_quantity(SD, "/quantity_gas.txt");
}

void check_internet() {
  if (!Ping.ping("www.google.co.th", 1)) {
    status_internet = false;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Blynk.config(auth, server, 8080);
    Serial.println(String(number) + " Ping Fail");
  } else {
    status_internet = true;
    Serial.println(String(number) + " Ping OK");
  }
}
