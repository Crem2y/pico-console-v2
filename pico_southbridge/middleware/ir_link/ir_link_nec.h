#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int decode_nec(const uint16_t* timing_buf, size_t timing_buf_size, uint8_t* data_buf, size_t data_buf_size);
int encode_nec(const uint8_t* data_buf, size_t data_buf_size, uint16_t* timing_buf, size_t timing_buf_size);

#ifdef __cplusplus
}
#endif