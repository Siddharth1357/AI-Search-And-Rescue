#include "MLX90640_I2C_Driver.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_NUM         I2C_NUM_0
#define I2C_SDA_PIN     8
#define I2C_SCL_PIN     9
#define I2C_FREQ_HZ     400000
#define I2C_TIMEOUT_MS  1000

static const char *TAG = "MLX_I2C";

void MLX90640_I2CInit(void) {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_PIN;
    conf.scl_io_num = I2C_SCL_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ_HZ;
    i2c_param_config(I2C_NUM, &conf);
    i2c_driver_install(I2C_NUM, conf.mode, 0, 0, 0);
    ESP_LOGI(TAG, "I2C initialized SDA=%d SCL=%d", I2C_SDA_PIN, I2C_SCL_PIN);
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress,
                     uint16_t nMemAddressRead, uint16_t *data) {
    uint8_t cmd[2] = {(uint8_t)(startAddress >> 8),
                      (uint8_t)(startAddress & 0xFF)};
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (slaveAddr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, cmd, 2, true);
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (slaveAddr << 1) | I2C_MASTER_READ, true);
    uint8_t *buf = (uint8_t*)malloc(nMemAddressRead * 2);
    i2c_master_read(handle, buf, nMemAddressRead * 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM, handle,
                        pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(handle);
    if (ret != ESP_OK) { free(buf); return -1; }
    for (int i = 0; i < nMemAddressRead; i++)
        data[i] = (buf[i*2] << 8) | buf[i*2+1];
    free(buf);
    return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress,
                      uint16_t data) {
    uint8_t cmd[4] = {
        (uint8_t)(writeAddress >> 8),
        (uint8_t)(writeAddress & 0xFF),
        (uint8_t)(data >> 8),
        (uint8_t)(data & 0xFF)
    };
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (slaveAddr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(handle, cmd, 4, true);
    i2c_master_stop(handle);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM, handle,
                        pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(handle);
    return (ret == ESP_OK) ? 0 : -1;
}

void MLX90640_I2CFreqSet(int freq) {}