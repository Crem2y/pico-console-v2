#include "ir_link.hpp"
#include "bridge_protocol.hpp"
#include <stdio.h> //for test

extern bridgeProtocol Bridge;

irLink::irLink(ir_pulse_capture_t* ir_rx, ir_tx_t* ir_tx) {
  _ir_rx = ir_rx;
  _ir_tx = ir_tx;
}

void irLink::init() {
  current_tx_format = IR_FORMAT_UNKNOWN;
  current_rx_format = IR_FORMAT_UNKNOWN;
  tx_data_len = 0;
}

void irLink::update(void) {
  //placeholder for testing ir_rx capture
  time_ms_t now_time = get_system_time_ms();
  static time_ms_t start_time;
  static ir_pulse_t p[IR_LINK_MAX_PULSES]; //test
  static int pulse_count = 0;

  if (pulse_count == 0) {
    start_time = now_time;
  }
  while(ir_pulse_capture_available(_ir_rx)) {
    if(pulse_count < IR_LINK_MAX_PULSES && ir_pulse_capture_read(_ir_rx, &p[pulse_count])) {
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

    size_t data_len;
    switch(current_rx_format) {
      case IR_FORMAT_MANUAL:
        data_len = valid_pulse_count * sizeof(uint16_t);
        memcpy(rx_data, rx_timings, data_len);
        break;
      case IR_FORMAT_NEC:
        data_len = decode_nec(rx_timings, valid_pulse_count, rx_data, IR_LINK_MAX_DATA_SIZE);
        break;
      default:
        // unknown format
        data_len = 0;
        break;
    }
    if(data_len) {
      set_bridge_rx_data(current_rx_format, rx_data, data_len);
    }
    pulse_count = 0;
  }
}

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

void irLink::send_blocking(const uint16_t* data, uint16_t len) {
  bool mark = true;
  enable_rx(false); // disable RX to avoid self-interference during transmission
  for(int i=0; i<len; i++) {
    if(mark) {
      ir_tx_write_mark_us(_ir_tx, data[i]);
    } else {
      ir_tx_write_space_us(_ir_tx, data[i]);
    }
    mark = !mark; // toggle between mark and space
  }
  enable_rx(true); // re-enable RX after transmission
}

void irLink::enable_tx(bool enable) {
  if(enable) {
    ir_tx_start(_ir_tx);
  } else {
    ir_tx_stop(_ir_tx);
  }
}

void irLink::enable_rx(bool enable) {
  if(enable) {
    ir_pulse_capture_start(_ir_rx);
  } else {
    ir_pulse_capture_stop(_ir_rx);
  }
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
  
  current_tx_format = (enum ir_format)data[0];
  uint8_t sequence_info = data[1];
  size_t sequence_index = (sequence_info >> 4) & 0x0F;
  size_t sequence_length = sequence_info & 0x0F;

  if(sequence_index == 0) {
    tx_data_len = 0; // reset data length for new sequence
    is_data_ready = false;
  }

  for(int i = 0; i < len - 2; i++) {
    tx_data[tx_data_len + i] = data[i+2];
  }
  tx_data_len += (len - 2);

  if(sequence_index != sequence_length) {
    return; // wait for more sequences
  }
  is_data_ready = true; // all sequences received

  switch(current_tx_format) {
    case IR_FORMAT_MANUAL:
      send_blocking(tx_timings, tx_timing_len);
      break;
    case IR_FORMAT_NEC:
      tx_timing_len = encode_nec(tx_data, tx_data_len, tx_timings, IR_LINK_MAX_PULSES);
      if(tx_timing_len > 0) {
        send_blocking(tx_timings, tx_timing_len);
      }
      break;
    default:
      // unknown format
      return;
  }
}

void irLink::set_bridge_rx_data(enum ir_format format, const uint8_t* data, size_t len) {
  if(len > IR_LINK_MAX_DATA_SIZE) {
    len = IR_LINK_MAX_DATA_SIZE; // Limit to max data size
  }
  size_t payload_size;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  const size_t data_per_packet = (PAYLOAD_MAX_SIZE - 2);
  const size_t sequence_length = len / data_per_packet + ((len % data_per_packet) != 0 ? 1 : 0);

  for(int sequence_index = 0; sequence_index < sequence_length; sequence_index++) {
    size_t remain = len - (sequence_index * data_per_packet);
    size_t send_count = remain > data_per_packet ? data_per_packet : remain;

    payload_buf[0] = (uint8_t)format;
    payload_buf[1] = (sequence_index << 4) | ((sequence_length-1) & 0x0F);

    for (size_t j = 0; j < send_count; j++) {
      payload_buf[2 + j] = data[sequence_index * data_per_packet + j];
    }

    size_t payload_size = 2 + send_count;
    Bridge.bridge_msg_push(CMD_IR_RX_DATA, payload_size, payload_buf);
  }
}