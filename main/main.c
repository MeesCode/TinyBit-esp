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

static const char *TAG = "ST7789";

uint8_t image[128 * 128 * 3]; // RGB888

void on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
    uint8_t r = rgba[0]; // 0 - 255
    uint8_t g = rgba[1]; // 0 - 255
    uint8_t b = rgba[2]; // 0 - 255
    // uint8_t a = rgba[3]; // 0: fully transparent, 255: fully opaque

    // if (a) printf("put pixel at (%d, %d) with color #%02x%02x%02x\n", x, y, r, g, b);
    // ESP_LOGI(TAG, "put pixel at (%d, %d) with color #%02x%02x%02x", (int)x, (int)y, (int)r, (int)g, (int)b);

    image[(y * 128 + x) * 3 + 0] = r; // Red
    image[(y * 128 + x) * 3 + 1] = g; // Green
    image[(y * 128 + x) * 3 + 2] = b; // Blue
}

void ST7789(void *pvParameters)
{
	TFT_t dev;

	// Change SPI Clock Frequency
	spi_clock_speed(80000000); // 80MHz

	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
	lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

  SD_Init();
  Flash_Searching();
  s_example_read_file("/sdcard/test.txt");

#if CONFIG_INVERSION
	ESP_LOGI(TAG, "Enable Display Inversion");
	//lcdInversionOn(&dev);
	lcdInversionOff(&dev);
#endif

  //lcdFillScreen(&dev, BLACK);
  lcdDrawSquare(&dev, 0, 0, CONFIG_WIDTH, CONFIG_HEIGHT, BLACK);
  ESP_LOGI(TAG, "Display Initialized");


  pngle_t *pngle = pngle_new();

  pngle_set_draw_callback(pngle, on_draw);

  FILE *fp = fopen("/sdcard/test.png", "rb");

  char buf[1024];
  int remain = 0;
  int len;
	while (!feof(fp)) {
		if (remain >= sizeof(buf)) {
			ESP_LOGE(__FUNCTION__, "Buffer exceeded");
			while(1) vTaskDelay(1);
		}

		int len = fread(buf + remain, 1, sizeof(buf) - remain, fp);
		if (len <= 0) {
			//printf("EOF\n");
			break;
		}

		int fed = pngle_feed(pngle, buf, remain + len);
		if (fed < 0) {
			ESP_LOGE(__FUNCTION__, "ERROR; %s", pngle_error(pngle));
			while(1) vTaskDelay(1);
		}

		remain = remain + len - fed;
		if (remain > 0) memmove(buf, buf + fed, remain);
	}

  ESP_LOGI(TAG, "PNG file read complete");
  fclose(fp);

  int dy = 1;
  int py = 20;

	while(1) {
		lcdDrawImage(&dev, image, 20, py, 128, 128);

    py += dy;
    if (py >= CONFIG_HEIGHT - 20 - 128 || py <= 20) {
      dy = -dy; // reverse direction
      ESP_LOGI(TAG, "Change direction, py=%d", py);
    }
	}

  free(image);
	pngle_destroy(pngle);

	// never reach here
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

void app_main(void)
{
	xTaskCreate(ST7789, "ST7789", 1024*6, NULL, 2, NULL);
}
