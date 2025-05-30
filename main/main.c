#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "st7789.h"
#include "SD_SPI.h"
#include "pngle.h"

#include "tinybit.h"

static const char *TAG = "ST7789";

void on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
	tinybit_feed_catridge(rgba, 1); // Feed the cartridge buffer with the pixel data
}

void ST7789(void *pvParameters)
{
	TFT_t dev;

	// Change SPI Clock Frequency
	spi_clock_speed(80000000); // 80MHz

	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
	lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);
    SD_Init();

	// set up button io
	gpio_set_direction(9, GPIO_MODE_INPUT);

    //lcdFillScreen(&dev, BLACK);
    lcdDrawSquare(&dev, 0, 0, CONFIG_WIDTH, CONFIG_HEIGHT, BLACK);
    ESP_LOGI(TAG, "Display Initialized");

	// Initialize TinyBit with buffers
	uint8_t *display_buffer;
	uint8_t *button_state;
	tinybit_init(&display_buffer, &button_state);

    ESP_LOGI(TAG, "Loading PNG file from SD card...");
    FILE *fp = fopen("/sdcard/flappy.png", "rb");

	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to open PNG file");
		return;
	}

    pngle_t *pngle = pngle_new();
    pngle_set_draw_callback(pngle, on_draw);
    
    char buf[1024];
    int remain = 0;
    int len;
	ESP_LOGI(TAG, "Init read buffer with size %zu", sizeof(buf));

	// Read the PNG file in chunks
	while (!feof(fp)) {
		if (remain >= sizeof(buf)) {
			ESP_LOGE(__FUNCTION__, "Buffer exceeded");
		}

		len = fread(buf + remain, 1, sizeof(buf) - remain, fp);
		if (len <= 0) {
			break;
		}

		int fed = pngle_feed(pngle, buf, remain + len);
		if (fed < 0) {
			ESP_LOGE(__FUNCTION__, "ERROR; %s", pngle_error(pngle));
		}

		remain = remain + len - fed;
		if (remain > 0) memmove(buf, buf + fed, remain);
	}

    ESP_LOGI(TAG, "PNG file read complete");
    fclose(fp);
    pngle_destroy(pngle);

	tinybit_start();
	ESP_LOGI(TAG, "TinyBit loaded game");

    // logic loop
	// int64_t start, end;
	while(1) {
        // start = esp_timer_get_time(); // Start time in microseconds
        tinybit_frame();
		// end = esp_timer_get_time(); // End time in microseconds
		// ESP_LOGI(TAG, "Frame render time: %lld us", (end - start));

		// read gpio 9
		if(!gpio_get_level(9)){
			*button_state |= (1 << 2) & 0xff;
		} else {
			*button_state &= ~(1 << 2) & 0xff;
		}

        lcdDrawImage(&dev, display_buffer, 20, 20, 128, 128);
    }

	// never reach here
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

void app_main(void)
{
	xTaskCreate(ST7789, "ST7789", 1024*6, NULL, 2, NULL);
}
