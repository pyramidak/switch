extern "C" {
#include "user_interface.h" //Required for LIGHT_SLEEP_T delay mode
} 

void light_sleep() {
  if (sensorDigital1 >= 1 and sensorDigital1 <= 6) {
    WiFi.mode(WIFI_OFF);
    delay(10);
    report("light sleep: ON", true);    
    if (sensorDigital1 >= 1 and sensorDigital1 <= 3) {
      gpio_pin_wakeup_enable(GPIO_ID_PIN(pinD1), GPIO_PIN_INTR_HILEVEL);    
    } else {
      gpio_pin_wakeup_enable(GPIO_ID_PIN(pinD1), GPIO_PIN_INTR_LOLEVEL);    
    }
    wifi_set_opmode(NULL_MODE);
    wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_set_wakeup_cb(light_wake);
    wifi_fpm_do_sleep(0xFFFFFFF);
    delay(1000);
  } else {
    report("no wakeup PIN ready - no sleep");  
  }
}

void light_wake() {
  Serial.flush();  
  report("light sleep: OFF");  
  mqtt_reconnectNeeded = true;
  wi_fi.begin(deviceName);
}