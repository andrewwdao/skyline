
void motor_init() {
	pinMode(INA_PIN, OUTPUT);
	pinMode(INB_PIN, OUTPUT);
	pinMode(EN_PIN, OUTPUT);
	
	digitalWrite(EN_PIN, LOW);
	digitalWrite(INA_PIN, LOW); 
  digitalWrite(INB_PIN, LOW);
	
  attachInterrupt(OPEN_LIMIT_PIN, open_limit_isr, RISING);
  attachInterrupt(CLOSE_LIMIT_PIN, close_limit_isr, RISING);

	// configure LED PWM functionalitites
  ledcSetup(CHANNEL, FREQ, RES);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PWM_PIN, CHANNEL);
	D_PRINTLN("Motor ready.");
}

void motor_stop() {
  D_PRINTLN("Motor stopped");
  digitalWrite(EN_PIN, LOW);
  digitalWrite(INA_PIN, LOW); 
  digitalWrite(INB_PIN, LOW);
  delay(100);
}
void motor_close() {
  if (!digitalRead(CLOSE_LIMIT_PIN)) { //if signal = LOW --> limit not triggered
    D_PRINTLN("Motor moving...Closing...");
    digitalWrite(EN_PIN, HIGH);
    digitalWrite(INA_PIN, HIGH); 
    digitalWrite(INB_PIN, LOW);

    int dutyCycle=0;
    for(;;){
      dutyCycle=(dutyCycle<255)?(dutyCycle+1):(dutyCycle);
      ledcWrite(CHANNEL, dutyCycle);// changing the PWM duty cycle
      if (LIMIT_FLAG) {
        GATE_STATE == CLOSED;
        LIMIT_FLAG = false;
        motor_stop();
        return;
      }
      delay(100);
    }
  } else { // if signal = HIGH --> limit reached, don't move motor
    D_PRINTLN("Limit reached! Door already closed.");
    GATE_STATE == CLOSED;
    return;
  }
}

void motor_open() {
  if (!digitalRead(OPEN_LIMIT_PIN)) { //if signal = LOW --> limit not triggered
    D_PRINTLN("Motor moving...Opening...");
    digitalWrite(EN_PIN, HIGH);
    digitalWrite(INA_PIN, LOW);
    digitalWrite(INB_PIN, HIGH);

    int dutyCycle=0;
    for(;;){
      dutyCycle=(dutyCycle<255)?(dutyCycle+1):(dutyCycle);
      ledcWrite(CHANNEL, dutyCycle);// changing the PWM duty cycle
      if (LIMIT_FLAG) {
        GATE_STATE == OPENED;
        LIMIT_FLAG = false;
        motor_stop();
        return;
      }
      delay(100);
    }
  } else { // if signal = HIGH --> limit reached, don't move motor
    D_PRINTLN("Limit reached! Door already opened.");
    GATE_STATE == OPENED;
    return;
  }
}
void Task_Motor( void * parameter ) {//task run on core0
  /***************************************SETUP************************************************/
  motor_init();
  xSemaphoreTake(baton, portMAX_DELAY); // ( TickType_t ) and portTICK_PERIOD_MS is also available , view: http://esp32.info/docs/esp_idf/html/d1/d19/group__xSemaphoreTake.html 
  xSemaphoreGive(baton);
  /***************************************LOOP************************************************/
  for (;;) {
    if ((GATE_STATE == CLOSED)||(GATE_STATE == OPENED)) {
      motor_stop();
    } else if (GATE_STATE == CLOSING) {
      motor_close();
    } else if (GATE_STATE == OPENING) {
      motor_open();
    }
  }
}
