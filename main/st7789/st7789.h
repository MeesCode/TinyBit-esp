#ifndef MAIN_ST7789_H_
#define MAIN_ST7789_H_

#include "driver/spi_master.h"

#define rgb565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define RED    rgb565(255,   0,   0) // 0xf800
#define GREEN  rgb565(  0, 255,   0) // 0x07e0
#define BLUE   rgb565(  0,   0, 255) // 0x001f
#define BLACK  rgb565(  0,   0,   0) // 0x0000
#define WHITE  rgb565(255, 255, 255) // 0xffff
#define GRAY   rgb565(128, 128, 128) // 0x8410
#define YELLOW rgb565(255, 255,   0) // 0xFFE0
#define CYAN   rgb565(  0, 156, 209) // 0x04FA
#define PURPLE rgb565(128,   0, 128) // 0x8010

#define CONFIG_WIDTH 172
#define CONFIG_HEIGHT 320
#define CONFIG_OFFSETX 34
#define CONFIG_OFFSETY 0
#define CONFIG_MOSI_GPIO 6
#define CONFIG_SCLK_GPIO 7
#define CONFIG_CS_GPIO 14
#define CONFIG_DC_GPIO 15
#define CONFIG_RESET_GPIO 21
#define CONFIG_BL_GPIO 22
#define HOST_ID SPI2_HOST

typedef struct {
	uint16_t _width;
	uint16_t _height;
	uint16_t _offsetx;
	uint16_t _offsety;
	int16_t _dc;
	int16_t _bl;
	spi_device_handle_t _SPIHandle;
} TFT_t;

void spi_clock_speed(int speed);
void spi_master_init(TFT_t * dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RESET, int16_t GPIO_BL);
bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t* Data, size_t DataLength);
bool spi_master_write_command(TFT_t * dev, uint8_t cmd);
bool spi_master_write_data_byte(TFT_t * dev, uint8_t data);
bool spi_master_write_data_word(TFT_t * dev, uint16_t data);
bool spi_master_write_addr(TFT_t * dev, uint16_t addr1, uint16_t addr2);

void delayMS(int ms);
void lcdInit(TFT_t * dev, int width, int height, int offsetx, int offsety);

bool spi_master_write_colors(TFT_t * dev, uint8_t * colors, uint16_t size);
bool spi_master_write_color(TFT_t * dev, uint16_t colors, uint16_t size);

void lcdDrawImage(TFT_t *dev, uint8_t *image, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void lcdDrawSquare(TFT_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
#endif /* MAIN_ST7789_H_ */

