#include "esp_check.h"
#include <stdint.h>
#include "driver/rmt_encoder.h"
#include "render_text.h"

/* #include <string.h> */
/* #include "freertos/FreeRTOS.h" */
/* #include "freertos/task.h" */
/* #include "esp_log.h" */
/* #include "driver/rmt_tx.h" */
/* #include "led_strip_encoder.h" */

static const char *TAG = "render_text";
/* static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3]; */

void set_px(uint8_t r, uint8_t c)
{
    uint8_t channels = 3;
    uint8_t cols = 8;

    uint8_t target = r*8*3 + c*3;

    led_strip_pixels[target] = 1;
    led_strip_pixels[target + 1] = 1;
    led_strip_pixels[target + 2] = 1;
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
        int len;
    } Pair;

    Pair data[9] = {{one_g, 8}, {two_g, 8}, {three_g, 8}, {four_g, 9}, {five_g, 8}, {six_g, 12}, {seven_g, 7}, {eight_g, 13}, {nine_g, 12}};

    int safe_c = (c - 1) % 10;
    render(data[safe_c].p, data[safe_c].len, start_x, start_y);
}

void fin_render()
{
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
    vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
    memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
}

void init_strip_or_something()
{
    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_channel_handle_t led_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    rmt_encoder_handle_t led_encoder = NULL;
    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    /* ESP_LOGI(TAG, "Enable RMT TX channel"); */
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    /* ESP_LOGI(TAG, "Start LED rainbow chase"); */
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };

    int count = 0;
    while (1) {
        /* --count; */

        /* count = count % 8; */

        render_char(8, 2, (count) % 8);
        render_char(9, 2, (count) % 8 + 4);

        /* led_strip_pixels[0] = 1; */
        /* led_strip_pixels[1] = 1; */
        /* led_strip_pixels[2] = 1; */

        /* for (int i = 0; i < 3; i++) { */
        /*     for (int j = i; j < EXAMPLE_LED_NUMBERS; j += 3) { */
        /*         // Build RGB pixels */
        /*         hue = j * 360 / EXAMPLE_LED_NUMBERS + start_rgb; */
        /*         led_strip_hsv2rgb(hue, 100, 5, &red, &green, &blue); */
        /*         led_strip_pixels[j * 3 + 0] = green; */
        /*         led_strip_pixels[j * 3 + 1] = blue; */
        /*         led_strip_pixels[j * 3 + 2] = red; */
        /*     } */
            // Flush RGB values to LEDs
}
