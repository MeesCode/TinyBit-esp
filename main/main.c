#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <dirent.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "st7789.h"
#include "SD_SPI.h"

#include "tinybit.h"

static const char *TAG = "ST7789";

struct TinyBitMemory tb_mem = {0};
uint8_t bs = 0;

void game_log(const char* c){
	ESP_LOGI("GAME", "%s", c);
}

int game_count_cb() {
    int count = 0;
    DIR *dir = opendir("/sdcard/");
    if (dir == NULL) {
        perror("opendir");
        return 0;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len >= 4 && strcmp(name + len - 4, ".PNG") == 0) {
            count++;
        }
    }
    closedir(dir);
    return count;
}

void game_load_cb(int index){

    uint8_t buffer[256];
    char filepath[256] = {0}; // Initialize to empty string

    // open directory
    DIR *dir = opendir("/sdcard/");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // find the file at the given index
    struct dirent *entry;
    int count = 0;
    bool found = false;
    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len >= 4 && strcmp(name + len - 4, ".PNG") == 0) {
            if (count == index) {
                snprintf(filepath, sizeof(filepath), "/sdcard/%.200s", name);
                found = true;
                break;
            }
            count++;
        }
    }
    closedir(dir);
    
    if (!found) {
        printf("Game index %d not found\n", index);
        return;
    }

    // load file
    FILE *fp = fopen(filepath, "rb");
    if (fp) {
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            tinybit_feed_cartridge(buffer, bytes_read);
        }
        fclose(fp);
    } else {
        printf("Failed to open file: %s\n", filepath);
    }
	
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
	gpio_set_direction(0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(0, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(1, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(19, GPIO_MODE_INPUT);
    gpio_set_pull_mode(19, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(18, GPIO_MODE_INPUT);
    gpio_set_pull_mode(18, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(23, GPIO_MODE_INPUT);
    gpio_set_pull_mode(23, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(20, GPIO_MODE_INPUT);
    gpio_set_pull_mode(20, GPIO_PULLDOWN_ONLY);

    //lcdFillScreen(&dev, BLACK);
    lcdDrawSquare(&dev, 0, 0, CONFIG_WIDTH, CONFIG_HEIGHT, BLACK);
    ESP_LOGI(TAG, "Display Initialized");

	// Initialize TinyBit with buffers
	tinybit_init(&tb_mem, &bs);

	ESP_LOGI(TAG, "TinyBit loaded game");
	tinybit_log_cb(game_log);
	tinybit_gamecount_cb(game_count_cb);
    tinybit_gameload_cb(game_load_cb);

	tinybit_start();

    // logic loop
	const int64_t target_frame_us = 16667; // 16.667 ms in microseconds
	int64_t frame_start;
	while(1) {
        frame_start = esp_timer_get_time(); // microseconds

        tinybit_frame();

		// read gpio 0
		if(gpio_get_level(0)){
			bs |= (1 << TB_BUTTON_B) & 0xff;
		} else {
			bs &= ~(1 << TB_BUTTON_B) & 0xff;
		}

        // read gpio 1
		if(gpio_get_level(1)){
			bs |= (1 << TB_BUTTON_A) & 0xff;
		} else {
			bs &= ~(1 << TB_BUTTON_A) & 0xff;
		}

        // read gpio 19
        if(gpio_get_level(19)){
            bs |= (1 << TB_BUTTON_LEFT) & 0xff;
        } else {
            bs &= ~(1 << TB_BUTTON_LEFT) & 0xff;
        }

        // read gpio 18
        if(gpio_get_level(18)){
            bs |= (1 << TB_BUTTON_UP) & 0xff;
        } else {
            bs &= ~(1 << TB_BUTTON_UP) & 0xff;
        }

        // read gpio 23
        if(gpio_get_level(23)){
            bs |= (1 << TB_BUTTON_DOWN) & 0xff;
        } else {
            bs &= ~(1 << TB_BUTTON_DOWN) & 0xff;
        }

        // read gpio 20
        if(gpio_get_level(20)){
            bs |= (1 << TB_BUTTON_RIGHT) & 0xff;
        } else {
            bs &= ~(1 << TB_BUTTON_RIGHT) & 0xff;
        }

        lcdDrawImage(&dev, tb_mem.display, 20, 20, 128, 128); // frame render time ~6910 us

        // Frame limiting to 60 FPS
        while ((esp_timer_get_time() - frame_start) < target_frame_us);
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
