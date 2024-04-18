#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c_master.h"

void app_main(void)
{
	i2c_master_bus_config_t i2cMasterCfg = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = I2C_NUM_0,
		.scl_io_num = 0,
		.sda_io_num = 1,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};

	i2c_master_bus_handle_t i2cBusHandle;
	ESP_ERROR_CHECK(i2c_new_master_bus(&i2cMasterCfg, &i2cBusHandle));

	i2c_device_config_t i2cDevCfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = 0x49,
		.scl_speed_hz = 50000,
	};

	i2c_master_dev_handle_t i2cDevHandle;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(i2cBusHandle, &i2cDevCfg, &i2cDevHandle));

 	/* Colocar aqui o código relativo ao teste a realizar */
}
