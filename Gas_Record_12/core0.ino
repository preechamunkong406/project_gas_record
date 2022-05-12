void Task0code(void * pvParameters) {
  if (!SD.begin(5)) Serial.println("Card Mount Failed");
  ds3231.begin();
  start_time();
  wg.begin(4, 0);
  read_file_quantity(SD, "/quantity_gas.txt");
  read_file_google_sheet(SD, "/google_sheet.txt");
  status_read_file_id = true;
  digitalWrite(LED_BUILTIN, LOW);

  for (;;) {
    ArduinoOTA.handle();
    slave.poll(modbus_rtu, 19);
    read_modbus_rtu();
    read_rfid();
    check_card();
    run_time();

    if (status_read_file_id) {
      status_read_file_id = false;
      for (file_id = 1; file_id < 101; ++file_id) {
        read_file_id();
      }
      status_read_file_id_ok = true;
      Serial.println("read_file_id 1-100 ok");
    }

    if (!digitalRead(relay_pin) && millis() > time_relay + time_unlock) {
      digitalWrite(relay_pin, HIGH);
      status_led_lock = false;
      Serial.println("Lock");
    }

    if (status_clear_hmi && status_hmi_touch) number_last = number;
    if (!status_clear_hmi) {
      if (long_wg_rfid_code > 0 || number_car > 0 || mileage > 0 || int_quantity_send > 0 || int_quantity_back > 0) {
        status_clear_hmi = true;
        number_last = number;
      }
    } else if (status_clear_hmi && number >= number_last + 180) {
      status_clear_hmi = false;
      number_last = number;
      modbus_rtu[7] = 0;
      modbus_rtu[8] = 0;
      modbus_rtu[9] = 0;
      modbus_rtu[10] = 0;
      modbus_rtu[11] = 0;
      modbus_rtu[13] = 0;
      modbus_rtu[14] = 0;
      Serial.println("clear_data_hmi");
    }

    if (int_screen_jump == 2 && !status_screen2) {
      number_last = number;
      status_screen2 = true;
    }

    if (long_wg_rfid_code == 0 && int_quantity_send == 0 && int_quantity_back == 0 && status_screen2 && number >= number_last + 180) {
      status_screen2 = false;
      modbus_rtu[18] = 1;
      Serial.println("screen_jump_1");
    }

    status_sw_door = digitalRead(sw_door_pin);
    if (!status_sw_door) couter_open_door = number;

    if (status_sw_door && !status_send_line && number >= couter_open_door + time_line_use && status_sw_send_line && !level_line_notify) {
      buffer_line_message = " " + string_value1 + "   " + string_value2 + "\nประตูยังไม่ได้ปิด เกินเวลา " + string_time_line_use + " นาที\nผู้ใช้งานล่าสุด: " + string_file_id + "." + string_value4;
      status_send_line = true;
      status_step_line = true;

    } else if (status_sw_door && !status_send_line && number >= couter_open_door + time_line_send && status_sw_send_line && level_line_notify) {
      buffer_line_message = " " + string_value1 + "   " + string_value2 + "\nประตูยังไม่ได้ปิด เกินเวลา " + string_time_line_send + " นาที\nผู้ใช้งานล่าสุด: " + string_file_id + "." + string_value4;
      status_send_line = true;
      status_step_line = true;

    } else if (!status_sw_door && status_send_line && status_step_line && status_sw_send_line) {
      buffer_line_message = " " + string_value1 + "   " + string_value2 + "\nประตู้ปิด เรียบร้อยเเล้วค่ะ";
      status_send_line_ok = false;
      status_step_line = false;
    }

    for (index_t = 4; index_t < 6; index_t++) {
      if (status_line_timer[index_t] && (int_hour == int_timer_hour[index_t]) && (int_minute == int_timer_minute[index_t])) {
        status_line_quantity = true;
        buffer_line_quantity_message =  " " + string_rct_date + "   " + string_int_timer[index_t] + "\nถังเเก๊สเต็ม  จำนวน  " + String(int_quantity_total) + " ถัง\nถังเเก๊สหมด จำนวน  " + String(int_quantity_empty) + " ถัง";
      }
    }

    if (status_line_quantity_ok && number > couter_line_quantity + 65) {
      status_line_quantity = false;
      status_line_quantity_ok = false;
    }

    if (status_read_file_google_sheet) {
      status_read_file_google_sheet = false;
      read_file_google_sheet(SD, "/google_sheet.txt");
    }
  }
}



void check_card() {
  if (status_sw_door_use && long_wg_rfid_code > 0 && number_car > 0 && mileage > 0) {
    for (file_id = 1; file_id < 101; ++file_id) {
      if (long_wg_rfid_code == long_sd_rfid_code[file_id]) {
        if (int_quantity_total > 0) {
          int_quantity_total--;
          int_quantity_empty++;
        }
        int_quantity_send = 0;
        int_quantity_back = 0;
        string_file_id = String(file_id);
        string_value1 = string_rct_date;
        string_value2 = string_rct_time;
        string_value3 = string_sd_rfid_code[file_id];
        string_value4 = string_sd_rfid_name[file_id];
        string_value5 = String(number_car);
        string_value6 = String(mileage);
        string_value7 = String(int_quantity_total);
        string_value8 = String(int_quantity_empty);
        string_value9 = String(int_quantity_send);
        string_value10 = String(int_quantity_back);
        string_blynk_value1 = string_rct_date;
        string_blynk_value2 = string_rct_time;
        string_blynk_value3 = string_sd_rfid_code[file_id];
        string_blynk_value4 = string_sd_rfid_name[file_id];
        string_blynk_value5 = String(number_car);
        string_blynk_value6 = String(mileage);
        string_blynk_value7 = String(int_quantity_total);
        string_blynk_value8 = String(int_quantity_empty);
        string_blynk_value9 = String(int_quantity_send);
        string_blynk_value10 = String(int_quantity_back);
        status_blynk_use = true;
        level_line_notify = false;
        action_unlock();
        write_sd_quantity();
        if (status_internet) {
          status_spreadsheet = true;
        } else {
          append_file_sd_google_sheet();
        }
      }
    }
    modbus_rtu[7] = 0;
    modbus_rtu[8] = 0;
    modbus_rtu[9] = 0;
    modbus_rtu[10] = 0;
    modbus_rtu[11] = 0;
  }

  if (status_sw_door_send && long_wg_rfid_code > 0 && int_quantity_send > 0) {
    for (file_id = 1; file_id < 101; ++file_id) {
      if (long_wg_rfid_code == long_sd_rfid_code[file_id] && int_sd_rfid_send[file_id]) {
        number_car = 0;
        mileage = 0;
        int_quantity_total = int_quantity_total + int_quantity_send;
        int_quantity_empty = int_quantity_empty - int_quantity_back;
        string_file_id = String(file_id);
        string_value1 = string_rct_date;
        string_value2 = string_rct_time;
        string_value3 = string_sd_rfid_code[file_id];
        string_value4 = string_sd_rfid_name[file_id];
        string_value5 = String(number_car);
        string_value6 = String(mileage);
        string_value7 = String(int_quantity_total);
        string_value8 = String(int_quantity_empty);
        string_value9 = String(int_quantity_send);
        string_value10 = String(int_quantity_back);
        string_blynk_value1 = string_rct_date;
        string_blynk_value2 = string_rct_time;
        string_blynk_value3 = string_sd_rfid_code[file_id];
        string_blynk_value4 = string_sd_rfid_name[file_id];
        string_blynk_value5 = String(number_car);
        string_blynk_value6 = String(mileage);
        string_blynk_value7 = String(int_quantity_total);
        string_blynk_value8 = String(int_quantity_empty);
        string_blynk_value9 = String(int_quantity_send);
        string_blynk_value10 = String(int_quantity_back);
        status_blynk_send = true;
        level_line_notify = true;
        action_unlock();
        write_sd_quantity();
        if (status_internet) {
          status_spreadsheet = true;
        } else {
          append_file_sd_google_sheet();
        }
      }
    }
    modbus_rtu[7] = 0;
    modbus_rtu[8] = 0;
    modbus_rtu[13] = 0;
    modbus_rtu[14] = 0;
  }
}

void action_unlock() {
  time_relay = millis();
  status_led_lock = true;
  status_send_line = false;
  status_send_line_ok = false;
  status_step_line = false;
  couter_open_door = number;
  digitalWrite(relay_pin, LOW);
  Serial.println("Unlock");
}

void read_modbus_rtu() {
  modbus_rtu[1] = int_day;
  modbus_rtu[2] = int_month;
  modbus_rtu[3] = int_year;
  modbus_rtu[4] = int_hour;
  modbus_rtu[5] = int_minute;
  modbus_rtu[6] = int_second;

  long_wg_rfid_code = (modbus_rtu[8] * 65536) + modbus_rtu[7];
  number_car = modbus_rtu[9];
  mileage = (modbus_rtu[11] * 65536) + modbus_rtu[10];
  status_sw_door_use = modbus_rtu[12];

  int_quantity_send = modbus_rtu[13];
  int_quantity_back = modbus_rtu[14];
  status_sw_door_send = modbus_rtu[15];
  modbus_rtu[16] = status_hmi_restart;
  status_hmi_touch = modbus_rtu[17];
  int_screen_jump = modbus_rtu[18];
}

void run_time() {
  if (millis() > time_run + 1000) {
    time_run = millis();
    if (status_update_ntp_ok) {
      ds3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
      ds3231.adjust(DateTime(int_set_year, int_set_month, int_set_day, int_set_hour, int_set_minute, int_set_second));
      status_update_ntp = false;
      status_update_ntp_ok = false;
      Serial.println("Update DS3231 OK");
    }
    start_time();
  }
}

void start_time() {
  DateTime now = ds3231.now();
  string_rct_year = String(now.year());
  if (now.month() < 10) {
    string_rct_month = "0" + String(now.month());
  } else {
    string_rct_month = now.month();
  }
  if (now.day() < 10) {
    string_rct_day = "0" + String(now.day());
  } else {
    string_rct_day = now.day();
  }
  if (now.hour() < 10) {
    string_rct_hour = "0" + String(now.hour());
  } else {
    string_rct_hour = now.hour();
  }
  if (now.minute() < 10) {
    string_rct_minute = "0" + String(now.minute());
  } else {
    string_rct_minute = now.minute();
  }
  if (now.second() < 10) {
    string_rct_second = "0" + String(now.second());
  } else {
    string_rct_second = now.second();
  }

  int_date = now.dayOfTheWeek();
  int_day = now.day();
  int_month = now.month();
  int_year = now.year();
  int_hour = now.hour();
  int_minute = now.minute();
  int_second = now.second();

  string_rct_date = string_rct_day + "/" + string_rct_month + "/" + string_rct_year;
  string_rct_date2 = String(daysOfTheWeek[now.dayOfTheWeek()]) + "  " + string_rct_day + "/" + string_rct_month + "/" + string_rct_year;
  string_rct_time = string_rct_hour + ":" + string_rct_minute + ":" + string_rct_second;
}

void update_ntp() {
  if (year() == 1970) {
    Serial.println("Failed NTP");
    return;
  }

  status_update_ntp = false;
  status_update_ntp_ok = true;
  int_set_day = day();
  int_set_month = month();
  int_set_year = year();
  int_set_hour = hour();
  int_set_minute = minute();
  int_set_second = second();

  Serial.print("Time Blynk: ");
  Serial.print(int_set_day);
  Serial.print("/");
  Serial.print(int_set_month);
  Serial.print("/");
  Serial.print(int_set_year);
  Serial.print("   ");
  Serial.print(int_set_hour);
  Serial.print(":");
  Serial.print(int_set_minute);
  Serial.print(":");
  Serial.println(int_set_second);
}

void write_sd_quantity() {
  String string_write_file_buffer_quantity;
  char char_write_file_buffer_quantity[50];
  string_write_file_buffer_quantity = String(int_quantity_total) + "\n" + String(int_quantity_empty);

  string_write_file_buffer_quantity.toCharArray(char_write_file_buffer_quantity, 50);
  writeFile(SD, "/quantity_gas.txt", char_write_file_buffer_quantity);
}

void read_file_quantity(fs::FS &fs, const char * path) {
  Serial.println();
  Serial.printf("%s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed File /quantity_gas.txt");
    return;
  }

  int status_code_quantity = 0;
  String string_buffer_quantity_total = "";
  String string_buffer_quantity_empty = "";

  while (file.available()) {
    char inChar = (char)file.read();
    if (inChar == '\n') status_code_quantity++;

    if (status_code_quantity == 0) {
      string_buffer_quantity_total += inChar;
    } else if (status_code_quantity == 1) {
      string_buffer_quantity_empty += inChar;
    }
  }

  string_quantity_total = string_buffer_quantity_total;
  int_quantity_total = string_quantity_total.toInt();

  string_buffer_quantity_empty.remove(0, 1);
  string_quantity_empty = string_buffer_quantity_empty;
  int_quantity_empty = string_quantity_empty.toInt();

  Serial.println(int_quantity_total);
  Serial.println(int_quantity_empty);
  file.close();
}

void append_file_sd_google_sheet() {
  String string_write_file_buffer;
  char char_write_file_buffer[100];
  string_write_file_buffer = "\n" + string_value1 + "," + string_value2 + "," + string_value3 + "," + string_value4 + "," + string_value5
                             + "," + string_value6 + "," + string_value7 + "," + string_value8 + "," + string_value9 + "," + string_value10;

  Serial.println(string_write_file_buffer);
  string_write_file_buffer.toCharArray(char_write_file_buffer, 100);
  appendFile(SD, "/google_sheet.txt", char_write_file_buffer);
}

void read_file_google_sheet(fs::FS &fs, const char * path) {
  Serial.println();
  Serial.printf("%s\n", path);

  for (gs_id = 0; gs_id < 501; ++gs_id) {
    string_sd_gs_all[gs_id] = "";
  }

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed File /google_sheet.txt");
    return;
  }

  gs_id = 0;
  while (file.available()) {
    char inChar = (char)file.read();
    if (inChar == '\n') gs_id++;
    if (gs_id > 500) break;
    string_sd_gs_all[gs_id] += inChar;
  }

  Serial.println("gs_id: " + String(gs_id));

  if (gs_id > 0) {
    gs_id_max_blynk = gs_id;
    status_blynk_gs_log = true;
  }

  gs_id_max = gs_id + 1;
  for (gs_id = 0; gs_id < gs_id_max; ++gs_id) {
    string_sd_gs_all[gs_id].remove(0, 1);
  }

  gs_id = 1;
  gs_id_send = 1;
  file.close();
}

void decoder_gs_text() {
  String BufferString = "", CalString = "";
  uint8_t NumText = 0;
  string_sd_gs_buffer_day = "";
  string_sd_gs_buffer_time = "";
  string_sd_gs_buffer_rfid = "";
  string_sd_gs_buffer_name = "";
  string_sd_gs_buffer_car = "";
  string_sd_gs_buffer_mileage = "";
  string_sd_gs_buffer_quantity_total = "";
  string_sd_gs_buffer_quantity_empty = "";
  string_sd_gs_buffer_quantity_send = "";
  string_sd_gs_buffer_quantity_back = "";

  BufferString = string_sd_gs_all[gs_id];
  //Serial.println(BufferString);

  CalString = BufferString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_day = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_time = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_rfid = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_name = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_car = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_mileage = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_quantity_total = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_quantity_empty = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_quantity_send = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_sd_gs_buffer_quantity_back = CalString;

  string_value1 = string_sd_gs_buffer_day;
  string_value2 = string_sd_gs_buffer_time;
  string_value3 = string_sd_gs_buffer_rfid;
  string_value4 = string_sd_gs_buffer_name;
  string_value5 = string_sd_gs_buffer_car;
  string_value6 = string_sd_gs_buffer_mileage;
  string_value7 = string_sd_gs_buffer_quantity_total;
  string_value8 = string_sd_gs_buffer_quantity_empty;
  string_value9 = string_sd_gs_buffer_quantity_send;
  string_value10 = string_sd_gs_buffer_quantity_back;
}

void read_file_id() {
  String string_file_buffer = "/rfid" + String(file_id) + ".txt";
  char char_file_buffer[15];
  string_file_buffer.toCharArray(char_file_buffer, 15);
  readFile(SD, char_file_buffer);
}

void readFile(fs::FS &fs, const char * path) {
  //  Serial.println();
  //  Serial.printf("%s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed File /rfid1.txt");
    return;
  }

  String string_buffer_rfid = "";
  String BufferString = "", CalString = "";
  uint8_t NumText = 0;
  String string_buffer_rfid_code = "", string_buffer_rfid_name = "", string_buffer_rfid_send = "";

  while (file.available()) {
    char inChar = (char)file.read();
    string_buffer_rfid += inChar;
  }

  BufferString = string_buffer_rfid;
  CalString = BufferString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_buffer_rfid_code = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_buffer_rfid_name = CalString;

  CalString = BufferString;
  CalString.remove(0, NumText + 1);
  BufferString = CalString;
  NumText = CalString.indexOf(",");
  CalString = CalString.substring(0, NumText);
  string_buffer_rfid_send = CalString;

  string_sd_rfid_code[file_id] = string_buffer_rfid_code;
  long_sd_rfid_code[file_id] = string_buffer_rfid_code.toDouble();
  string_sd_rfid_name[file_id] = string_buffer_rfid_name;
  string_sd_rfid_send[file_id] = string_buffer_rfid_send;
  int_sd_rfid_send[file_id] = string_buffer_rfid_send.toInt();

  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void read_rfid() {
  if (wg.available()) {
    long_wg_rfid_code = (wg.getCode());
    modbus_rtu[7] = long_wg_rfid_code;
    modbus_rtu[8] = long_wg_rfid_code / 65535;
    Serial.println("DECIMAL = " + String(long_wg_rfid_code));
  }
}

void couter() {
  number++;
  //Serial.println(number);
  if (status_internet) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    delay(5);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
