#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"

#include "st7789.h"

#define TAG "ST7789"
#define	_DEBUG_ 0

#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST
#endif

#define SPI_DEFAULT_FREQUENCY SPI_MASTER_FREQ_20M; // 20MHz

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode = 1;

int clock_speed_hz = SPI_DEFAULT_FREQUENCY;

void spi_clock_speed(int speed) {
	ESP_LOGI(TAG, "SPI clock speed=%d MHz", speed/1000000);
	clock_speed_hz = speed;
}

void spi_master_init(TFT_t * dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RESET, int16_t GPIO_BL)
{
	esp_err_t ret;

	ESP_LOGI(TAG, "GPIO_CS=%d",GPIO_CS);
	if ( GPIO_CS >= 0 ) {
		//gpio_pad_select_gpio( GPIO_CS );
		gpio_reset_pin( GPIO_CS );
		gpio_set_direction( GPIO_CS, GPIO_MODE_OUTPUT );
		gpio_set_level( GPIO_CS, 0 );
	}

	ESP_LOGI(TAG, "GPIO_DC=%d",GPIO_DC);
	//gpio_pad_select_gpio( GPIO_DC );
	gpio_reset_pin( GPIO_DC );
	gpio_set_direction( GPIO_DC, GPIO_MODE_OUTPUT );
	gpio_set_level( GPIO_DC, 0 );

	ESP_LOGI(TAG, "GPIO_RESET=%d",GPIO_RESET);
	if ( GPIO_RESET >= 0 ) {
		//gpio_pad_select_gpio( GPIO_RESET );
		gpio_reset_pin( GPIO_RESET );
		gpio_set_direction( GPIO_RESET, GPIO_MODE_OUTPUT );
		gpio_set_level( GPIO_RESET, 1 );
		delayMS(100);
		gpio_set_level( GPIO_RESET, 0 );
		delayMS(100);
		gpio_set_level( GPIO_RESET, 1 );
		delayMS(100);
	}

	ESP_LOGI(TAG, "GPIO_BL=%d",GPIO_BL);
	if ( GPIO_BL >= 0 ) {
		//gpio_pad_select_gpio(GPIO_BL);
		gpio_reset_pin(GPIO_BL);
		gpio_set_direction( GPIO_BL, GPIO_MODE_OUTPUT );
		gpio_set_level( GPIO_BL, 0 );
	}

	ESP_LOGI(TAG, "GPIO_MOSI=%d",GPIO_MOSI);
	ESP_LOGI(TAG, "GPIO_SCLK=%d",GPIO_SCLK);
	spi_bus_config_t buscfg = {
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = -1,
		.sclk_io_num = GPIO_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4096,
		.flags = 0
	};

	ret = spi_bus_initialize( HOST_ID, &buscfg, SPI_DMA_CH_AUTO );
	ESP_LOGD(TAG, "spi_bus_initialize=%d",ret);
	assert(ret==ESP_OK);

	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));
	//devcfg.clock_speed_hz = SPI_Frequency;
	devcfg.clock_speed_hz = clock_speed_hz;
	devcfg.queue_size = 7;
	//devcfg.mode = 2;
	devcfg.mode = 3;
	devcfg.flags = SPI_DEVICE_NO_DUMMY;

	if ( GPIO_CS >= 0 ) {
		devcfg.spics_io_num = GPIO_CS;
	} else {
		devcfg.spics_io_num = -1;
	}
	
	spi_device_handle_t handle;
	ret = spi_bus_add_device( HOST_ID, &devcfg, &handle);
	ESP_LOGD(TAG, "spi_bus_add_device=%d",ret);
	assert(ret==ESP_OK);
	dev->_dc = GPIO_DC;
	dev->_bl = GPIO_BL;
	dev->_SPIHandle = handle;
}

bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t* Data, size_t DataLength)
{
	spi_transaction_t SPITransaction;
	esp_err_t ret;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Data;
#if 1
		ret = spi_device_transmit( SPIHandle, &SPITransaction );
#else
		ret = spi_device_polling_transmit( SPIHandle, &SPITransaction );
#endif
		assert(ret==ESP_OK); 
	}

	return true;
}

bool spi_master_write_command(TFT_t * dev, uint8_t cmd)
{
	static uint8_t Byte = 0;
	Byte = cmd;
	gpio_set_level( dev->_dc, SPI_Command_Mode );
	return spi_master_write_byte( dev->_SPIHandle, &Byte, 1 );
}

bool spi_master_write_data_byte(TFT_t * dev, uint8_t data)
{
	static uint8_t Byte = 0;
	Byte = data;
	gpio_set_level( dev->_dc, SPI_Data_Mode );
	return spi_master_write_byte( dev->_SPIHandle, &Byte, 1 );
}

bool spi_master_write_addr(TFT_t * dev, uint16_t addr1, uint16_t addr2)
{
	static uint8_t Byte[4];
	Byte[0] = (addr1 >> 8) & 0xFF;
	Byte[1] = addr1 & 0xFF;
	Byte[2] = (addr2 >> 8) & 0xFF;
	Byte[3] = addr2 & 0xFF;
	gpio_set_level( dev->_dc, SPI_Data_Mode );
	return spi_master_write_byte( dev->_SPIHandle, Byte, 4);
}

bool spi_master_write_colors(TFT_t * dev, uint8_t * colors, uint16_t size)
{
	static uint8_t Byte[4096];
	int index = 0;
	for(int i=0;i<size;i++) {
		// convert to rgb565
		uint16_t rgb565 = rgb565(colors[i*3], colors[i*3+1], colors[i*3+2]);
		Byte[index++] = (rgb565 >> 8) & 0xFF;
		Byte[index++] = rgb565 & 0xFF;
	}
	gpio_set_level( dev->_dc, SPI_Data_Mode );
	return spi_master_write_byte( dev->_SPIHandle, Byte, size*2);
}

bool spi_master_write_color(TFT_t * dev, uint16_t color, uint16_t size)
{
	static uint8_t Byte[4096];
	int index = 0;
	for(int i=0;i<size;i++) {
		Byte[index++] = (color >> 8) & 0xFF;
		Byte[index++] = color & 0xFF;
	}
	gpio_set_level( dev->_dc, SPI_Data_Mode );
	return spi_master_write_byte( dev->_SPIHandle, Byte, size*2);
}

void delayMS(int ms) {
	int _ms = ms + (portTICK_PERIOD_MS - 1);
	TickType_t xTicksToDelay = _ms / portTICK_PERIOD_MS;
	ESP_LOGD(TAG, "ms=%d _ms=%d portTICK_PERIOD_MS=%"PRIu32" xTicksToDelay=%"PRIu32,ms,_ms,portTICK_PERIOD_MS,xTicksToDelay);
	vTaskDelay(xTicksToDelay);
}

void lcdInit(TFT_t * dev, int width, int height, int offsetx, int offsety)
{
	dev->_width = width;
	dev->_height = height;
	dev->_offsetx = offsetx;
	dev->_offsety = offsety;

	spi_master_write_command(dev, 0x01);	//Software Reset
	delayMS(150);

	spi_master_write_command(dev, 0x11);	//Sleep Out
	delayMS(255);
	
	spi_master_write_command(dev, 0x3A);	//Interface Pixel Format
	spi_master_write_data_byte(dev, 0x55);
	delayMS(10);
	
	spi_master_write_command(dev, 0x36);	//Memory Data Access Control
	spi_master_write_data_byte(dev, 0x00);

	spi_master_write_command(dev, 0x2A);	//Column Address Set
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0xF0);

	spi_master_write_command(dev, 0x2B);	//Row Address Set
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0x00);
	spi_master_write_data_byte(dev, 0xF0);

	spi_master_write_command(dev, 0x21);	//Display Inversion On
	delayMS(10);

	spi_master_write_command(dev, 0x13);	//Normal Display Mode On
	delayMS(10);

	spi_master_write_command(dev, 0x29);	//Display ON
	delayMS(255);

	if(dev->_bl >= 0) {
		gpio_set_level( dev->_bl, 1 );
	}
}

// Draw Frame Buffer
void lcdDrawImage(TFT_t *dev, uint8_t *image, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	spi_master_write_command(dev, 0x2A); // set column(x) address
	spi_master_write_addr(dev, dev->_offsetx+x, dev->_offsetx+x+w-1);
	spi_master_write_command(dev, 0x2B); // set Page(y) address
	spi_master_write_addr(dev, dev->_offsety+y, dev->_offsety+y+h-1);
	spi_master_write_command(dev, 0x2C); // Memory Write

	//uint16_t size = dev->_width*dev->_height;
	uint32_t size = w*h;
	while (size > 0) {
		// 1365 pixels per time (4095 bytes RGB).
		uint16_t bs = (size > 2048) ? 2048 : size;
		spi_master_write_colors(dev, image, bs);
		size -= bs;
		image += bs*3; // 3 bytes per pixel
	}
	return;
}

// Draw Frame Buffer filled with a single color
void lcdDrawSquare(TFT_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	spi_master_write_command(dev, 0x2A); // set column(x) address
	spi_master_write_addr(dev, dev->_offsetx+x, dev->_offsetx+x+w-1);
	spi_master_write_command(dev, 0x2B); // set Page(y) address
	spi_master_write_addr(dev, dev->_offsety+y, dev->_offsety+y+h-1);
	spi_master_write_command(dev, 0x2C); // Memory Write

	//uint16_t size = dev->_width*dev->_height;
	uint32_t size = w*h;
	while (size > 0) {
		// 1365 pixels per time (4095 bytes RGB).
		uint16_t bs = (size > 2048) ? 2048 : size;
		spi_master_write_color(dev, color, bs);
		size -= bs;
	}
	return;
}

//TODO: draw square in single color, for example for filling the screen