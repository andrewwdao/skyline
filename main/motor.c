/*------------------------------------------------------------*-
  Brushed DC motor controller - source file
  (c) Minh-An Dao - Nhu-Phung Tran-Thi 2020
  version 1.10 - 20/08/2020
---------------------------------------------------------------
 * Use MCPWM module to control brushed dc motor.
 * 
 *  ** CAUTION:
 *  This core is responsible for RF radio, Wifi stack and protocols (watchdog,...) 
 *  Therefore, you have to be EXTREMELY CAREFUL when create task for this core,
 *  in order to not intefere these default functions (watchdog has the piority of 1 among 0 to N, higher number means higer piority)
 *
 --------------------------------------------------------------*/
#ifndef __MOTOR_C
#define __MOTOR_C
#include "motor.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#include "freertos/queue.h"
#include "driver/gpio.h"

// ------ Private constants -----------------------------------
// --- pinout define ---
#define PWM_PIN 27 //Set GPIO as PWM0A
#define INA_PIN 25
#define INB_PIN 26
#define EN_PIN  33
#define MOTOR_PIN_SEL  ((1ULL<<EN_PIN)|(1ULL<<INA_PIN)|(1ULL<<INB_PIN))
  
// isr pins
#define OPEN_LIMIT_PIN  22
#define CLOSE_LIMIT_PIN 23
#define OPEN_DOOR_PIN 32
#define CLOSE_DOOR_PIN 35
#define STOP_DOOR_PIN 34
#define ISR_PIN_SEL  ((1ULL<<OPEN_LIMIT_PIN)|(1ULL<<CLOSE_LIMIT_PIN)|(1ULL<<OPEN_DOOR_PIN)|(1ULL<<CLOSE_DOOR_PIN)|(1ULL<<STOP_DOOR_PIN))
#define OPEN_RF_PIN 5
#define CLOSE_RF_PIN 18
#define STOP_RF_PIN 19
#define RF_PIN_SEL  ((1UL<<OPEN_RF_PIN)|(1ULL<<CLOSE_RF_PIN)|(1ULL<<STOP_RF_PIN))
// --- parameters define ---
// setting PWM properties
#define FREQ    10000
#define MAX_SPEED CONFIG_MAX_SPEED
#define MIN_SPEED CONFIG_MIN_SPEED
#define LIMIT_INTERVAL_MS CONFIG_LIMIT_INTERVAL
// ------ Private function prototypes -------------------------
static void motor_init(void);
static void motor_stop(void);
static void motor_open(void);
static void motor_close(void);
// ------ Private variables -----------------------------------
static xQueueHandle gpio_evt_queue = NULL;
/** @brief tag used for ESP serial console messages */
static const char TAG[] = "motor";
// ------ PUBLIC variable definitions -------------------------

//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
/**
 * @brief Interrupt service routine for buttons and limit switches
*/
static void IRAM_ATTR gpio_isr(void* arg)
{
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
/**
 * @brief Interrupt task for buttons and limit switches
*/
static void motor_isr_task(void* arg)
{
    uint32_t io_num;
    for(;;)
    {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            //ESP_LOGW(TAG, "GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if ((io_num==OPEN_LIMIT_PIN)&&(!gpio_get_level(OPEN_LIMIT_PIN)))
            {
              STOP_FLAG = true;
              GATE_STATE = OPENED;
              ESP_LOGW(TAG, "OPEN LIMIT isr!\n");
            }
            if ((io_num==CLOSE_LIMIT_PIN)&&(!gpio_get_level(CLOSE_LIMIT_PIN)))
            {
              STOP_FLAG = true;
              GATE_STATE = CLOSED;
              ESP_LOGW(TAG, "CLOSE LIMIT isr!\n");
            }
            if (((io_num==STOP_DOOR_PIN)&&(!gpio_get_level(STOP_DOOR_PIN)))||
                ((io_num==STOP_RF_PIN)&&(gpio_get_level(STOP_RF_PIN))))
            {
              STOP_FLAG = true;
              if (!((GATE_STATE==OPENED)|(GATE_STATE==CLOSED)))
              {GATE_STATE = STOPPED;}
              ESP_LOGW(TAG, "STOP isr!\n");
            }
            if (((io_num==OPEN_DOOR_PIN)&&(!gpio_get_level(OPEN_DOOR_PIN)))||
                ((io_num==OPEN_RF_PIN)&&(gpio_get_level(OPEN_RF_PIN))))
            {
              if (GATE_STATE == CLOSING) {
                    STOP_FLAG = true;
                    GATE_STATE = STOPPED;
                }
              DELAY_MS(800);
              GATE_STATE = OPENING;
              ESP_LOGW(TAG, "OPEN isr!\n");
            }
            if (((io_num==CLOSE_DOOR_PIN)&&(!gpio_get_level(CLOSE_DOOR_PIN)))||
                ((io_num==CLOSE_RF_PIN)&&(gpio_get_level(CLOSE_RF_PIN))))
            {
              if (GATE_STATE == OPENING) {
                    STOP_FLAG = true;
                    GATE_STATE = STOPPED;
                }
              DELAY_MS(800);
              GATE_STATE = CLOSING;
              ESP_LOGW(TAG, "CLOSE isr!\n");
            }
            DELAY_MS(10);
        }
    }
}

/**
 * @brief Configure MCPWM module for brushed dc motor and isr for buttons and limit switches
*/
static void motor_init(void)
{
    ESP_LOGI(TAG, "initializing gpio...\n");
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = MOTOR_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //set initial values for gpio
    gpio_set_level(EN_PIN, LOW);
    gpio_set_level(INA_PIN, LOW);
    gpio_set_level(INB_PIN, LOW);

    //1. mcpwm gpio initialization
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PWM_PIN);
    //2. initial mcpwm configuration
    ESP_LOGI(TAG, "Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = FREQ;  //frequency in Hz,
    pwm_config.cmpr_a = 0;         //duty cycle of PWMxA = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A with above settings
    
    //set initial value for pwm
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    
    ESP_LOGI(TAG, "Motor ready.\n");

    // --- limit switches and physical buttons declaration
    //interrupt of falling edge
    io_conf.intr_type = GPIO_PIN_INTR_NEGEDGE;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins
    io_conf.pin_bit_mask = ISR_PIN_SEL;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    // --- RF buttons declaration
    //interrupt of rising edge - since the RF module using reversed logic
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins
    io_conf.pin_bit_mask = RF_PIN_SEL;
    //enable pull-down mode
    io_conf.pull_down_en = 1;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(1, sizeof(uint32_t)); // uxQueueLength, uxItemSize
    //start gpio task - https://www.freertos.org/a00125.html
    xTaskCreate(
      motor_isr_task,                     /* Task Function */
      "motor_isr_task",                   /* Name of Task */
      2048,                               /* Stack size of Task */
      NULL,                               /* Parameter of the task */
      10,                                 /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
      NULL);                              /* Task handle to keep track of created task */

    //install gpio isr service
    gpio_uninstall_isr_service(); // clean up any left over
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1); //set a level 5 piority interrupt (7 is the highest)
    // //hook isr handler for specific gpio pins
    gpio_isr_handler_add(OPEN_LIMIT_PIN, gpio_isr, (void*) OPEN_LIMIT_PIN);
    gpio_isr_handler_add(CLOSE_LIMIT_PIN, gpio_isr, (void*) CLOSE_LIMIT_PIN);
    gpio_isr_handler_add(OPEN_DOOR_PIN, gpio_isr, (void*) OPEN_DOOR_PIN);
    gpio_isr_handler_add(CLOSE_DOOR_PIN, gpio_isr, (void*) CLOSE_DOOR_PIN);
    gpio_isr_handler_add(STOP_DOOR_PIN, gpio_isr, (void*) STOP_DOOR_PIN);
    gpio_isr_handler_add(OPEN_RF_PIN, gpio_isr, (void*) OPEN_RF_PIN);
    gpio_isr_handler_add(CLOSE_RF_PIN, gpio_isr, (void*) CLOSE_RF_PIN);
    gpio_isr_handler_add(STOP_RF_PIN, gpio_isr, (void*) STOP_RF_PIN);
    ESP_LOGI(TAG, "ISR for Limit switches and buttons ready.\n");
}

/**
 * @brief stop motor func
 */
static void motor_stop(void)
{
  if (gpio_get_level(EN_PIN)| //if motor is still active, then stop it
      gpio_get_level(INA_PIN)|
      gpio_get_level(INB_PIN))
  {
    gpio_set_level(EN_PIN, LOW);
    gpio_set_level(INA_PIN, LOW);
    gpio_set_level(INB_PIN, LOW);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    ESP_LOGW(TAG, "Motor stopped\n");
  }
  DELAY_MS(100);
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void motor_open(void)
{
    if (gpio_get_level(OPEN_LIMIT_PIN)) //if signal = HIGH --> limit not triggered
    {
      ESP_LOGW(TAG, "Open command. Motor moving...\n");
      gpio_set_level(INA_PIN, LOW);
      gpio_set_level(INB_PIN, HIGH);
      gpio_set_level(EN_PIN, HIGH);
      
      int dutyCycle=MIN_SPEED;
      int64_t millis = esp_timer_get_time();
      uint8_t time_limiter = 0;
      for(;;) {
      dutyCycle=(dutyCycle<MAX_SPEED)?(dutyCycle+1):(dutyCycle);
      mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, dutyCycle);
      mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
      time_limiter = (esp_timer_get_time()-millis)>LIMIT_INTERVAL_MS; 
      if (STOP_FLAG | time_limiter) {
            motor_stop();
            STOP_FLAG = false;
            if (time_limiter) GATE_STATE = STOPPED;
            return;
        }
      DELAY_MS(100);
      }
    } else { // if signal = LOW --> limit reached, don't move motor
      ESP_LOGW(TAG, "Limit reached! Door already opened.\n");
      GATE_STATE = OPENED;
      return;
    }
}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void motor_close(void)
{
    if (gpio_get_level(CLOSE_LIMIT_PIN)) //if signal = HIGH --> limit not triggered
    {
        ESP_LOGW(TAG, "Close command. Motor moving...\n");
        gpio_set_level(INA_PIN, HIGH);
        gpio_set_level(INB_PIN, LOW);
        gpio_set_level(EN_PIN, HIGH);
        
        int dutyCycle=MIN_SPEED;
        int64_t millis = esp_timer_get_time();
        uint8_t time_limiter = 0;
        for(;;)
        {
          dutyCycle=(dutyCycle<MAX_SPEED)?(dutyCycle+1):(dutyCycle);
          // ledcWrite(CHANNEL, dutyCycle);// changing the PWM duty cycle
          mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, dutyCycle);
          mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
          time_limiter = (esp_timer_get_time()-millis)>LIMIT_INTERVAL_MS; 
          if (STOP_FLAG | time_limiter) {
            motor_stop();
            STOP_FLAG = false;
            if (time_limiter) GATE_STATE = STOPPED;
            return;
        }
          DELAY_MS(100);
        }
    } else // if signal = LOW --> limit reached, don't move motor
    {
        ESP_LOGW(TAG, "Limit reached! Door already closed.\n");
        GATE_STATE = CLOSED;
        return;
    }
}

/**
 * @brief Motor task to run on main
 */
void motor_task(void *arg)
{
  /***************************************SETUP************************************************/
  //mcpwm gpio initialization and configuration
  motor_init();
  /***************************************LOOP************************************************/
  for (;;) 
  {
    if (GATE_STATE == CLOSING) {motor_close();}
    else if (GATE_STATE == OPENING) {motor_open();}
    motor_stop();
  }
}

#endif