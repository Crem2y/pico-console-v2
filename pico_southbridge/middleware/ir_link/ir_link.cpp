#include "ir_link.hpp"
#include "bridge_protocol.hpp"
#include <stdio.h> //for test

extern bridgeProtocol Bridge;

irLink::irLink(ir_pulse_capture_t* ir_cap) {
  _ir_cap = ir_cap;
}

void irLink::init() {
  current_tx_format = IR_FORMAT_UNKNOWN;
  current_rx_format = IR_FORMAT_UNKNOWN;
}

void irLink::update(void) {
  //placeholder for testing ir_rx capture
  time_ms_t now_time = get_system_time_ms();
  static time_ms_t start_time;
  static ir_pulse_t p[256]; //test
  static int pulse_count = 0;

  if (pulse_count == 0) {
    start_time = now_time;
  }
  while(ir_pulse_capture_available(_ir_cap)) {
    if(pulse_count < 256 && ir_pulse_capture_read(_ir_cap, &p[pulse_count])) {
      pulse_count++;
    }
  }
  if(system_time_elapsed_ms(now_time, start_time) > 1000 && pulse_count > 0) {
    printf("Captured %d IR pulses\n", pulse_count);
    for(int i=0; i<pulse_count; i++) {
      printf("Pulse %3d: Level=%d, Duration=%dus\n", i, p[i].level, p[i].duration_us);
    }

    int valid_pulse_count = 0;
    for(int i=1; i<pulse_count; i++) { // start from 1 to skip the first pulse
      if(p[i].duration_us > 65535) {
        break; // skip pulses that exceed max duration for 16-bit timing
      }
      rx_timings[i-1] = p[i].duration_us;
      valid_pulse_count++;
    }
    if(current_rx_format == IR_FORMAT_MANUAL) {
      set_bridge_rx_data(IR_FORMAT_MANUAL, (uint8_t*)rx_timings, valid_pulse_count * sizeof(uint16_t));
    } else if(current_rx_format == IR_FORMAT_NEC) {
      if(parse_nec_data()) {
        printf("Captured nec data:\n");
        printf("%02X %02X %02X %02X\n", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
        set_bridge_rx_data(IR_FORMAT_NEC, (uint8_t*)rx_data, 4);
      }
    }
    pulse_count = 0;
  }
}

int irLink::parse_nec_data(void) {
  // check start pulse
  if(rx_timings[0] < 8500 || rx_timings[0] > 10000) {
    printf("Invalid start pulse\n");
    return 0; // invalid start pulse
  }
  // check space pulse
  if(rx_timings[1] < 4000 || rx_timings[1] > 5500) {
    printf("Invalid space pulse\n");
    return 0; // invalid space pulse
  }
  // parse data pulses
  for(int i=0; i<32; i++) {
    if(rx_timings[2 + (i*2)] < 300 || rx_timings[2 + (i*2)] > 700) {
      printf("Invalid data pulse at index %d: %dus\n", i, rx_timings[2 + (i*2)]);
      return 0; // invalid data pulse
    }
    rx_data[i / 8] <<= 1;
    if(rx_timings[2 + (i*2)+1] > 1000) {
      rx_data[i / 8] |= 1; // bit is 1
    }
    // else bit is 0, already shifted in
  }

  return 4; // valid NEC data length (4 bytes)
}

void irLink::enable_tx(bool enable) {

}

void irLink::enable_rx(bool enable) {

}

void irLink::get_bridge_enable_tx(const uint8_t* data, uint8_t len) {
  if (len < 1) return; // Not enough data
  enable_tx(true);
  current_tx_format = (enum ir_format)data[0];
}

void irLink::get_bridge_enable_rx(const uint8_t* data, uint8_t len) {
  if (len < 1) return; // Not enough data
  enable_rx(true);
  current_rx_format = (enum ir_format)data[0];
}

void irLink::get_bridge_tx_data(const uint8_t* data, uint8_t len) {
  if (len < 2) return; // Not enough data
  //placeholder
}

void irLink::set_bridge_rx_data(enum ir_format format, uint8_t* data, uint8_t len) {
  int payload_size;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  if(format == IR_FORMAT_MANUAL) {
    if(len > IR_LINK_MAX_DATA_SIZE) {
      len = IR_LINK_MAX_DATA_SIZE; // Limit to max data size
    }
    uint16_t* rx_timings_ptr = (uint16_t*)data;
    uint8_t pulse_count = len / sizeof(uint16_t);
    const uint8_t pulses_per_packet = (PAYLOAD_MAX_SIZE - 2) / sizeof(uint16_t);
    uint8_t sequence_length = pulse_count / pulses_per_packet + ((pulse_count % pulses_per_packet) != 0 ? 1 : 0);

    for(int sequence_index = 0; sequence_index < sequence_length; sequence_index++) {
      uint8_t remain = pulse_count - sequence_index * pulses_per_packet;
      uint8_t send_count = remain > pulses_per_packet ? pulses_per_packet : remain;

      payload_buf[0] = (uint8_t)format;
      payload_buf[1] = (sequence_index << 4) | (sequence_length & 0x0F);

      for (uint8_t j = 0; j < send_count; j++) {
        uint16_t timing = rx_timings_ptr[sequence_index * pulses_per_packet + j];
        
        // little endian format for 16-bit timing
        payload_buf[2 + j * 2]     = timing & 0xFF;
        payload_buf[2 + j * 2 + 1] = (timing >> 8) & 0xFF;
      }

      uint8_t payload_size = 2 + send_count * sizeof(uint16_t);
      Bridge.bridge_msg_push(CMD_IR_RX_DATA, payload_size, payload_buf);
    }
  } else if(format == IR_FORMAT_NEC) {
    payload_size = 6; // 1 byte format + 1 byte sequencing + 4 bytes NEC data
    if(len < 4) {
      return; // Not enough data
    }
    payload_buf[0] = (uint8_t)format;
    payload_buf[1] = 0x00;
    for(int i=0; i<4; i++) {
      payload_buf[2 + i] = data[i];
    }
    Bridge.bridge_msg_push(CMD_IR_RX_DATA, payload_size, payload_buf);
  }
}