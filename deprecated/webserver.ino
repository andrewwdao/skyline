/*  ESP32 Core0 function files
 *  (c) Minh-An Dao 2018
 *  
 *  CAUTION:
 *  this core is responsible for RF radio, Wifi stack and protocols (watchdog,...) 
 *  (the Arduino code is compiled to run on the second core (core_1). FreeRTOS manages everything)
 *  Therefore, you have to be EXTREMELY CAREFUL when create task for this core,
 *  in order to not intefere these default functions (watchdog has the piority of 1 among 0 to N, higher number means higer piority)
 *  
 *  P/S CAUTION: must include config.h in the main .ino file
 */

void Wifi_begin() {
  // Connect to WiFi access point.
  D_PRINTLN(); D_PRINTLN();
  D_PRINT(F("Connecting to "));
  D_PRINT(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    D_PRINT(F("."));
  }//endif
  
  D_PRINTLN(F("WiFi connected"));
  D_PRINTLN(F("IP address: "));D_PRINTLN(WiFi.localIP());
  D_PRINTLN();
}//end wifi begin

void webserver_init() {
  server.on("/skyline", handle_root);
  server.on("/get_state", get_state);
  server.on("/rotate", rotate_motor);
  server.begin();
  D_PRINTLN("HTTP server started");
  
}

void handle_root() { // Handle home url (/skyline)
  server.send(200, "text/html", HTML);
}

void get_state() { // Handle get_state url (/get_state)
  server.send(200, "application/json", gate_state);
}

void rotate_motor() { // Handle rotate url (/rotate)
  if (gate_state == CLOSED) {
    gate_state = OPENING;
  } else if (gate_state == OPENED) {
    gate_state = CLOSING;
  } else if (gate_state == CLOSING) {
    gate_state = OPENED;
  } else if (gate_state == OPENING) {
    gate_state = CLOSED;
  }
  D_PRINTLN("Command Received!");
  server.send(200, "application/json", gate_state);
}
