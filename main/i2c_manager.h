#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include <stddef.h>

#define I2C_MASTER_SDA 21
#define I2C_MASTER_SCL 22
#define I2C_PORT I2C_NUM_0

void i2c_manager_init(void);
void scan_i2c(char* out, size_t max_len);

#endif // I2C_MANAGER_H
