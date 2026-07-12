#include "bridge_control.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

bridgeControl::bridgeControl(void) {

}

void bridgeControl::init(void) {
  connected = false;
  last_bridge_comm_time = 0;
}

void bridgeControl::update(void) {
  connected = (system_time_elapsed_ms(get_system_time_ms(), last_bridge_comm_time) < 500);
}

void bridgeControl::send_bridge_hw_info_res(void) {
  int payload_size = 6;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  memcpy(&payload_buf[0], &my_info.hw_ver, sizeof(uint16_t));
  memcpy(&payload_buf[2], &my_info.hw_support, sizeof(uint32_t));

  Bridge.send(CMD_HW_INFO_RES, payload_size, payload_buf);
}

void bridgeControl::send_bridge_hw_name_res(void) {
  int payload_size = strlen(my_info.hw_name);
  uint8_t payload_buf[PAYLOAD_MAX_SIZE] = {0,};

  if(payload_size > PAYLOAD_MAX_SIZE) {
    payload_size = PAYLOAD_MAX_SIZE;
  }
  strncpy((char *)payload_buf, my_info.hw_name, payload_size);
  if(payload_size == PAYLOAD_MAX_SIZE) {
    payload_buf[PAYLOAD_MAX_SIZE-1] = '\0';
  }

  Bridge.send(CMD_HW_NAME_RES, payload_size, payload_buf);
}

void bridgeControl::send_bridge_sw_info_res(void) {
  int payload_size = 12;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  memcpy(&payload_buf[0], &my_info.sw_ver, sizeof(uint16_t));
  memcpy(&payload_buf[2], &my_info.build_date, 3);
  memcpy(&payload_buf[5], &my_info.build_time, 3);
  memcpy(&payload_buf[8], &my_info.sw_support, sizeof(uint32_t));

  Bridge.send(CMD_SW_INFO_RES, payload_size, payload_buf);
}