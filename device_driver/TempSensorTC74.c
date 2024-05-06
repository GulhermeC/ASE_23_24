#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "TempSensorTC74.h"

esp_err_t tc74_init(i2c_master_bus_handle_t* pBusHandle,
					i2c_master_dev_handle_t* pSensorHandle,
					uint8_t sensorAddr, int sdaPin, int sclPin, uint32_t clkSpeedHz)
{
	i2c_master_bus_config_t i2cMasterCfg = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = I2C_NUM_0,
		.scl_io_num = sclPin,
		.sda_io_num = sdaPin,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};

	esp_err_t err = i2c_new_master_bus(&i2cMasterCfg, pBusHandle);
    if (err != ESP_OK) return err;

	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = sensorAddr,
		.scl_speed_hz = clkSpeedHz ,
	};

	err = i2c_master_bus_add_device(*pBusHandle, &dev_cfg, pSensorHandle);
    if (err != ESP_OK) return err;

	return ESP_OK;
}

esp_err_t tc_74_free(i2c_master_bus_handle_t busHandle, i2c_master_dev_handle_t sensorHandle) {
    esp_err_t err;

    // Remove the device from the I2C bus
    if (sensorHandle != NULL) {
        err = i2c_master_bus_rm_device(sensorHandle);
        if (err != ESP_OK) {
            ESP_LOGE("TC74_FREE", "Failed to remove device from bus: %s", esp_err_to_name(err));
            return err;
        }
    }

    // Delete the I2C master bus
    if (busHandle != NULL) {
        err = i2c_del_master_bus(busHandle);
        if (err != ESP_OK) {
            ESP_LOGE("TC74_FREE", "Failed to delete I2C bus: %s", esp_err_to_name(err));
            return err;
        }
    }

    return ESP_OK;
}

esp_err_t tc74_standy(i2c_master_dev_handle_t sensorHandle)
{
	uint8_t buffer[2] = {0x01, 0x80};
	ESP_ERROR_CHECK(i2c_master_transmit(sensorHandle, buffer, sizeof(buffer), -1));
	vTaskDelay(10);

	return ESP_OK;
}

esp_err_t tc74_wakeup(i2c_master_dev_handle_t sensorHandle)
{
	uint8_t buffer[2] = {0x01, 0x00};
	ESP_ERROR_CHECK(i2c_master_transmit(sensorHandle, buffer, sizeof(buffer), -1));
	vTaskDelay(10);

	return ESP_OK;
}

bool tc74_is_temperature_ready(i2c_master_dev_handle_t sensorHandle)
{
	uint8_t status;
    ESP_ERROR_CHECK(i2c_master_receive(sensorHandle, &status, sizeof(status), -1));
    return (status & 0x40) != 0;
}

esp_err_t tc74_wakeup_and_read_temp(i2c_master_dev_handle_t sensorHandle, uint8_t* pTemp)
{
    esp_err_t err = tc74_wakeup(sensorHandle);
    if (err != ESP_OK) return err;

    while (!tc74_is_temperature_ready(sensorHandle)) {
        vTaskDelay(10);
    }

    return i2c_master_receive(sensorHandle, pTemp, 1, -1);
}

esp_err_t tc74_read_temp_after_cfg(i2c_master_dev_handle_t sensorHandle, uint8_t* pTemp)
{
	uint8_t read = 0x00;
	i2c_master_transmit(sensorHandle, &read, sizeof(read), -1);
    return i2c_master_receive(sensorHandle, pTemp, 1, -1);
}


esp_err_t tc74_read_temp_after_temp(i2c_master_dev_handle_t sensorHandle, uint8_t* pTemp)
{
    return i2c_master_receive(sensorHandle, pTemp, 1, -1); 
}