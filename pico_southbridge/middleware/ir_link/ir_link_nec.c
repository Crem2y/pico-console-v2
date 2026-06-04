#include "ir_link_nec.h"

int irLink::decode_nec(const uint16_t* timing_buf, size_t timing_buf_size, uint8_t* data_buf, size_t data_buf_size) {
  if(timing_buf_size < 68 || data_buf_size < 4) return 0; // not enough data for NEC
  // check start pulse
  if(timing_buf[0] < 8500 || timing_buf[0] > 10000) {
    return 0; // invalid start pulse
  }
  // check space pulse
  if(timing_buf[1] < 4000 || timing_buf[1] > 5500) {
    return 0; // invalid space pulse
  }
  // parse data pulses
  for(int i=0; i<32; i++) {
    if(timing_buf[2 + (i*2)] < 300 || timing_buf[2 + (i*2)] > 700) {
      return 0; // invalid data pulse
    }
    data_buf[i / 8] <<= 1;
    if(timing_buf[2 + (i*2)+1] > 1000) {
      data_buf[i / 8] |= 1; // bit is 1
    }
    // else bit is 0, already shifted in
  }
  // check stop bit
  if(timing_buf[66] < 300 || timing_buf[66] > 700) {
    return 0; // invalid stop pulse
  }

  return 4; // valid NEC data length (4 bytes)
}

int irLink::encode_nec(const uint8_t* data_buf, size_t data_buf_size, uint16_t* timing_buf, size_t timing_buf_size) {
  if(timing_buf_size < 68 || data_buf_size < 4) return 0; // not enough data for NEC

  timing_buf[0] = 9000; // start pulse
  timing_buf[1] = 4500; // space pulse
   // data pulses
  for(int i=0; i<32; i++) {
    timing_buf[2 + (i*2)] = 560;
    if((data_buf[i / 8] << (i % 8)) & 0x80) {
      timing_buf[2 + (i*2)+1] = 1690; // bit is 1
    } else {
      timing_buf[2 + (i*2)+1] = 560; // bit is 0
    }
  }
  timing_buf[66] = 560; // stop pulse
  timing_buf[67] = 560;
  return 68; // total number of timing entries for NEC
}