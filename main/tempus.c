// Big ol todo list
// 1. clean up this main, get it compiling with a minium of code
// 2. add some vc, dump to git.
// 3. integrate the rgb module. get the text rendering working.
// 4. get the wifi example working. from static config file.
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

/* #include "esp_console.h" */
#include "esp_log.h"
#include "temperature.h"
#include "secrets.h"

static const char *TAG = "tempus";

    // mqtt
    // DVES_USER
    // pwd intoMQTT
    // 1883, core-mosquitto
    // 192.168.1.111 # hass/


void app_main(void)
{
    ESP_LOGD(TAG, "main running");
    temp_reader_app();
}

