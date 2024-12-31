// Big ol todo list
// 3. get the wifi example working. from static config file.
// 4. integrate the rgb module. get the text rendering working.
// 5. add the mqtt layer on top. get test publishes.
// 6. rejig into two projects. (1) that publishes designs, (2) for matrix that just recieves pixel loads and renders
//
// after that it's optional content
// 1. get a live updating temp measure from mqtt
// 2. render album art in 8x8 and get it updating.
#include <stdio.h>
// maybe?
/* #include <inttypes.h> */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

/* #include "esp_console.h" */
#include "esp_log.h"
#include "secrets.h"
#include "wifi.h"
#include "mqtt.h"
#include "led.h"

/* #include "temperature.h" */
/* temp_reader_app(); */

/* static enum State { */
/*     STARTUP = 1, */
/*     WIFI_CONNECTING, */
/*     MQTT_CONNECTING, */
/*     COOKING */
/* }; */

static const char *TAG = "tempus";

    // mqtt
    // DVES_USER
    // pwd intoMQTT
    // 1883, core-mosquitto
    // 192.168.1.111 # hass/

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    /* enum state = STARTUP; */
    ESP_LOGD(TAG, "main running");

    setup_wifi();
    setup_led();
    setup_mqtt(set_led);
    /* setup_wifi(callback_for_wifi_done); */

    // how to stop this running and exiting? could block on wifi but really just want to chill and wait fo msg to come back.
    while (true)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

