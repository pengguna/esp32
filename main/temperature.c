#include <stdio.h>

// needed for main zzz
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "temperature.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static int chip_addr = 0x4a;
static int data_addr = 0x00;
static int reg_len = 2;

static const i2c_port_t i2c_port = I2C_NUM_0;
static const int i2c_gpio_sda = 18;
static const int i2c_gpio_scl = 19;

static uint32_t i2c_frequency = 100 * 1000;
static const char *TAG = "i2c_temp";

static i2c_master_bus_config_t i2c_bus_config;
static i2c_master_bus_handle_t tool_bus_handle;

static i2c_device_config_t i2c_dev_conf;
static i2c_master_dev_handle_t dev_handle;
static uint32_t I2C_TOOL_TIMEOUT_VALUE_MS = 50;


void temp_reader_app(void)
{
    if (!setup_i2c_temp())
    {
        ESP_LOGE(TAG, "No soup for you");
        return;
    }

    uint8_t data [2];

    while (true)
    {

        if (read_i2c_temp(&data[0]))
        {

            uint16_t res = ( data[0] << 8 ) | data[1];
            printf("raw: 0x%04x ", res);
            double conv = res * 0.0078125;
            printf("convert: %f\n", conv);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

bool setup_i2c_temp(void)
{
    // should this be spewed into global ns?
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = i2c_port,
        .scl_io_num = i2c_gpio_scl,
        .sda_io_num = i2c_gpio_sda,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    if (i2c_new_master_bus(&i2c_bus_config, &tool_bus_handle) != ESP_OK) {
        ESP_LOGE(TAG, "could not setup master bus");
        return false;
    }

    i2c_device_config_t i2c_dev_conf = {
        .scl_speed_hz = i2c_frequency,
        .device_address = chip_addr,
    };

    if (i2c_master_bus_add_device(tool_bus_handle, &i2c_dev_conf, &dev_handle) != ESP_OK) {
        ESP_LOGE(TAG, "could not setup dev bus");
        return false;
    }
    return true;
}


bool read_i2c_temp(uint8_t* data_ptr)
{
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, (uint8_t*)&data_addr, 1, data_ptr, reg_len, I2C_TOOL_TIMEOUT_VALUE_MS);
    /* esp_err_t ret = i2c_master_transmit_receive(dev_handle, (uint8_t*)&data_addr, 1, data, reg_len, I2C_TOOL_TIMEOUT_VALUE_MS); */

    if (ret == ESP_OK) {
        return true;


    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "Bus is busy");
        return false;
    }

    ESP_LOGW(TAG, "Read failed");
    return false;

}


