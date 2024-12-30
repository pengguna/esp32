#pragma once
#include <stdbool.h>

void temp_reader_app(void);
bool setup_i2c_temp(void);
bool read_i2c_temp(uint8_t* data_ptr);
