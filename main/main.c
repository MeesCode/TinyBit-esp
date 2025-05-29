#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"

#include "st7789.h"
#include "SD_SPI.h"
#include "pngle.h"

#include "tinybit.h"

static const char *TAG = "ST7789";

uint8_t image[128 * 128 * 4]; // RGB8888

void on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
    image[(y * 128 + x) * 3 + 0] = rgba[0]; // Red
    image[(y * 128 + x) * 3 + 1] = rgba[1]; // Green
    image[(y * 128 + x) * 3 + 2] = rgba[2]; // Blue
	image[(y * 128 + x) * 3 + 3] = rgba[3]; // Alpha (not used)
}

void ST7789(void *pvParameters)
{
	TFT_t dev;

	// Change SPI Clock Frequency
	spi_clock_speed(80000000); // 80MHz

	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
	lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);
    SD_Init();

    //lcdFillScreen(&dev, BLACK);
    lcdDrawSquare(&dev, 0, 0, CONFIG_WIDTH, CONFIG_HEIGHT, BLACK);
    ESP_LOGI(TAG, "Display Initialized");

    ESP_LOGI(TAG, "Loading PNG file from SD card...");
    FILE *fp = fopen("/sdcard/test.png", "rb");

    pngle_t *pngle = pngle_new();
    pngle_set_draw_callback(pngle, on_draw);
    
    char buf[1024];
    int remain = 0;
    int len;
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

    int dy = 1;
    int py = 20;

    // logic loop
	while(1) {
		lcdDrawImage(&dev, image, 20, py, 128, 128);

        py += dy;
        if (py >= CONFIG_HEIGHT - 20 - 128 || py <= 20) {
            dy = -dy; // reverse direction
        }
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
