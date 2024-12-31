/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "led.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      14

#define EXAMPLE_LED_NUMBERS         64 
#define EXAMPLE_CHASE_SPEED_MS      200

static const char *TAG = "led_module";

typedef struct {
    uint8_t x;
    uint8_t y;
} Point;

static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3];

static rmt_tx_channel_config_t tx_chan_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
    .gpio_num = RMT_LED_STRIP_GPIO_NUM,
    .mem_block_symbols = 64, // increase the block size can make the LED less flickering
    .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
    .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
};

static rmt_channel_handle_t led_chan = NULL;

static led_strip_encoder_config_t encoder_config = {
    .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
};

static rmt_encoder_handle_t led_encoder = NULL;

static rmt_transmit_config_t tx_config = {
    .loop_count = 0, // no transfer loop
};

void flush_px(void)
{
        ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
        /* vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS)); */
        /* memset(led_strip_pixels, 0, sizeof(led_strip_pixels)); */
        /* ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config)); */
        /* ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY)); */
}

void set_led(int loc, int val)
{
    ESP_LOGI(TAG, "WANT TO SET loc=%d to val=%d", loc, val);
    led_strip_pixels[loc] = val;
    flush_px();
}

void set_px(uint8_t r, uint8_t c)
{
    /* uint8_t channels = 3; */
    /* uint8_t cols = 8; */

    uint8_t target = r*8*3 + c*3;

    led_strip_pixels[target] = 1;
    led_strip_pixels[target + 1] = 1;
    led_strip_pixels[target + 2] = 1;
    
    flush_px();
}

void render(Point *points, size_t len, uint8_t start_r, uint8_t start_c) 
{
    int row_adj = 0;
    for (int i = 0; i < len; i++)
    {
    // problem: wrapped text will shift down a row
    // solution?
    // need to work out the starting pixel, and every pixel beyond the line needs to be pulled back
        /* int left_in_row = 8 - (start_c % 8); */
        /* if (i > left_in_row) */
        /* { */
        /*     row_adj = -1; */
        /* } */
        set_px(points[i].x + start_r + row_adj, points[i].y + start_c);
    }
}
// how long to read the 3 accelerators + one gyro take?
void render_char(int c, uint8_t start_x, uint8_t start_y)
{
    Point one_g[8] = {{0,1}, {1,0}, {1,1}, {2,1}, {3, 1}, {4, 0}, {4, 1}, {4,2}};
    Point two_g[8] = {{0,1}, {1,0}, {1,2}, {2,2}, {3, 1}, {4, 0}, {4, 1}, {4,2}};
    Point three_g[8] = {{0,0}, {0,1}, {1,2}, {2,0}, {2, 1}, {3, 2}, {4, 0}, {4,1}};
    Point four_g[9] = {{0,0}, {0,2}, {1,0}, {1,2}, {2, 0}, {2, 1}, {2, 2}, {3,2}, {4, 2}};
    Point five_g[8] = {{0,0}, {0,1}, {0,2}, {1,0}, {2, 1}, {3, 2}, {4, 0}, {4,1}};
    Point six_g[12] = {{0,0}, {0,1}, {0,2}, {1,0}, {2, 0}, {2, 1}, {2, 2}, {3,0}, {3, 2}, {4, 0}, {4,1},{4,2}};
    Point seven_g[7] = {{0,0}, {0,1}, {0,2}, {1,2}, {2, 1}, {3, 0}, {4, 0}};
    Point eight_g[13] = {{0,0}, {0,1}, {0,2}, {1,0}, {1,2}, {2, 0}, {2, 1}, {2, 2}, {3,0}, {3, 2}, {4, 0}, {4,1},{4,2}};
    Point nine_g[12] = {{0,0}, {0,1}, {0,2}, {1,0}, {2, 0}, {2, 1}, {2, 2}, {1,2}, {3, 2}, {4, 0}, {4,1},{4,2}};

    typedef struct {
        Point* p;
        int len; } Pair;

    Pair data[9] = {{one_g, 8}, {two_g, 8}, {three_g, 8}, {four_g, 9}, {five_g, 8}, {six_g, 12}, {seven_g, 7}, {eight_g, 13}, {nine_g, 12}};

    int safe_c = (c - 1) % 10;
    render(data[safe_c].p, data[safe_c].len, start_x, start_y);
}

void setup_led(void)
{
    ESP_LOGI(TAG, "Create RMT TX channel");
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
}

