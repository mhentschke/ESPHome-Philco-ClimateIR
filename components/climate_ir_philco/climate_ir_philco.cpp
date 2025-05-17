//#include "coolix.h"
#include "climate_ir_philco.h"
//#include "esphome/components/remote_base/coolix_protocol.h"
#include "philco_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace philco {

static const char *const TAG = "philco.climate";

static const uint32_t COOLIX_OFF = 0xB27BE0;
static const uint32_t COOLIX_SWING = 0xB26BE0;
static const uint32_t COOLIX_LED = 0xB5F5A5;
static const uint32_t COOLIX_SILENCE_FP = 0xB5F5B6;

// On, 25C, Mode: Auto, Fan: Auto, Zone Follow: Off, Sensor Temp: Ignore.
static const uint8_t COOLIX_COOL = 0b0000;
static const uint8_t COOLIX_DRY_FAN = 0b0100;
static const uint8_t COOLIX_AUTO = 0b1000;
static const uint8_t COOLIX_HEAT = 0b1100;
static const uint32_t COOLIX_MODE_MASK = 0b1100;
static const uint32_t COOLIX_FAN_MASK = 0xF000;
static const uint32_t COOLIX_FAN_MODE_AUTO_DRY = 0x1000;
static const uint32_t COOLIX_FAN_AUTO = 0xB000;
static const uint32_t COOLIX_FAN_MIN = 0x9000;
static const uint32_t COOLIX_FAN_MED = 0x5000;
static const uint32_t COOLIX_FAN_MAX = 0x3000;

// Temperature
//static const uint8_t COOLIX_TEMP_RANGE = COOLIX_TEMP_MAX - COOLIX_TEMP_MIN + 1;
static const uint8_t COOLIX_FAN_TEMP_CODE = 0b11100000;  // Part of Fan Mode.
static const uint32_t COOLIX_TEMP_MASK = 0b11110000;
/*static const uint8_t COOLIX_TEMP_MAP[COOLIX_TEMP_RANGE] = {
    0b00000000,  // 17C
    0b00010000,  // 18c
    0b00110000,  // 19C
    0b00100000,  // 20C
    0b01100000,  // 21C
    0b01110000,  // 22C
    0b01010000,  // 23C
    0b01000000,  // 24C
    0b11000000,  // 25C
    0b11010000,  // 26C
    0b10010000,  // 27C
    0b10000000,  // 28C
    0b10100000,  // 29C
    0b10110000   // 30C
};*/
static const uint8_t PHILCO_TEMP_RANGE = PHILCO_TEMP_MAX - PHILCO_TEMP_MIN + 1;
static const uint8_t PHILCO_TEMP_MAP_COOL[PHILCO_TEMP_RANGE] = {
    0b00011111,  // 16C
    0b00100000,  // 17c
    0b00100001,  // 18C
    0b00100010,  // 19C
    0b00010100,  // 20C
    0b00010101,  // 21C
    0b00010110,  // 22C
    0b00010111,  // 23C
    0b00011000,  // 24C
    0b00011001,  // 25C
    0b00011010,  // 26C
    0b00011011,  // 27C
    0b00011100,  // 28C
    0b00011101,  // 29C
    0b00011110,  // 30C
    0b00011111,  // 31C
    0b00100000   // 32C
};
static const uint8_t PHILCO_TEMP_MAP_HEAT[PHILCO_TEMP_RANGE] = {
    0b00100010,  // 16C
    0b00100011,  // 17c
    0b00100100,  // 18C
    0b00100101,  // 19C
    0b00010111,  // 20C
    0b00011000,  // 21C
    0b00011001,  // 22C
    0b00011010,  // 23C
    0b00011011,  // 24C
    0b00011100,  // 25C
    0b00011101,  // 26C
    0b00011110,  // 27C
    0b00011111,  // 28C
    0b00100000,  // 29C
    0b00100001,  // 30C
    0b00100010,  // 31C
    0b00100011   // 32C
};
static const uint8_t PHILCO_TEMP_MAP_AUTO[PHILCO_TEMP_RANGE] = {
    0b00100001,  // 16C
    0b00100010,  // 17c
    0b00100011,  // 18C
    0b00100100,  // 19C
    0b00010110,  // 20C
    0b00010111,  // 21C
    0b00011000,  // 22C
    0b00011001,  // 23C
    0b00011010,  // 24C
    0b00011011,  // 25C
    0b00011100,  // 26C
    0b00011101,  // 27C
    0b00011110,  // 28C
    0b00011111,  // 29C
    0b00100000,  // 30C
    0b00100001,  // 31C
    0b00100010   // 32C
};

static const uint32_t PHILCO_LOW_BASE = 0x6056;
static const uint32_t PHILCO_HIGH_BASE = 0x0000;
//static const uint8_t PHILCO_TEMP_MIN = 16;
//static const uint8_t PHILCO_TEMP_MAX = 32;
static const uint32_t PHILCO_TEMP_MIN_CODE = 0xC00; // in LSB
static const uint32_t PHILCO_TEMP_INCREMENT = 0x0100;
static const uint32_t PHILCO_FAN_TEMP_CODE = 0x1400;

// All Modes are in MSB
static const uint32_t PHILCO_AUTO = 0x0040;
static const uint32_t PHILCO_COOL = 0x0020;
static const uint32_t PHILCO_DRY = 0x0030;
static const uint32_t PHILCO_FAN = 0x0050;
static const uint32_t PHILCO_HEAT = 0x400010;
static const uint32_t PHILCO_FAN_AUTO = 0x0000;
static const uint32_t PHILCO_FAN_LOW = 0x0002;
static const uint32_t PHILCO_FAN_MED = 0x0003;
static const uint32_t PHILCO_FAN_HIGH = 0x0001;
static const uint8_t PHILCO_FAN_OFFSET = 0;
static const uint32_t PHILCO_OFF = 0xC000;
static const uint64_t PHILCO_OFF2 = (uint64_t)(0x0023) << 48;
static const uint32_t PHILCO_OFF_TEMP = 0x1300;
static const uint32_t PHILCO_SWING = 0x0002;
static const uint32_t PHILCO_SWING_OFF = 0x0000;
static const uint32_t PHILCO_SWING_OFFSET = 8;








void PhilcoClimate::transmit_state() {
  //uint32_t remote_state = 0xB20F00;
  uint32_t remote_state_low = PHILCO_LOW_BASE;
  uint32_t remote_state_high = PHILCO_HIGH_BASE;
  uint8_t swing_state2 = 0;
  uint8_t fan_state2 = 0;
  esphome::remote_base::PhilcoData remote_state;

  if (send_swing_cmd_) {
    send_swing_cmd_ = false;
    remote_state_high |= PHILCO_SWING << PHILCO_SWING_OFFSET;
    swing_state2 = PHILCO_SWING;
  }
  auto temp = (uint8_t) roundf(clamp<float>(this->target_temperature, PHILCO_TEMP_MIN, PHILCO_TEMP_MAX));
  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:
      remote_state_high |= PHILCO_COOL;
      remote_state.data2 = (PHILCO_TEMP_MAP_COOL[temp - PHILCO_TEMP_MIN]);
      remote_state.data2 = remote_state.data2 << 48;
      break;
    case climate::CLIMATE_MODE_HEAT:
      remote_state_high |= PHILCO_HEAT;
      remote_state.data2 = (PHILCO_TEMP_MAP_HEAT[temp - PHILCO_TEMP_MIN]);
      remote_state.data2 = remote_state.data2 << 48;
      break;
    case climate::CLIMATE_MODE_HEAT_COOL:
      remote_state_high |= PHILCO_AUTO;
      remote_state.data2 = (PHILCO_TEMP_MAP_AUTO[temp - PHILCO_TEMP_MIN]);
      remote_state.data2 = remote_state.data2 << 48;
      break;
    case climate::CLIMATE_MODE_FAN_ONLY:
      remote_state_high |= PHILCO_FAN;
      remote_state.data2 = 0;
      break;
    case climate::CLIMATE_MODE_DRY:
      remote_state_high |= PHILCO_DRY;
      remote_state.data2 = 0;
      break;
    case climate::CLIMATE_MODE_OFF:
    default:
      remote_state_high |= PHILCO_OFF;
      remote_state.data2 = PHILCO_OFF2;
      break;
  }
  ESP_LOGE(TAG, "Mode: 0x%08" PRIX32, this->mode);
  if (this->mode != climate::CLIMATE_MODE_OFF) {
    ESP_LOGE(TAG, "Entering Modes");
    if (this->mode != climate::CLIMATE_MODE_FAN_ONLY) {
      remote_state_low |= PHILCO_TEMP_MIN_CODE + (temp-PHILCO_TEMP_MIN) * PHILCO_TEMP_INCREMENT;
      ESP_LOGE(TAG, "Temp code_low: 0x%08" PRIX32, remote_state_low);
    } else {
      remote_state_low |= PHILCO_FAN_TEMP_CODE;
      ESP_LOGE(TAG, "Fan Temp code_low: 0x%08" PRIX32, remote_state_low);
    }
    if (this->mode == climate::CLIMATE_MODE_HEAT_COOL || this->mode == climate::CLIMATE_MODE_DRY) {
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
      //remote_state |= COOLIX_FAN_MODE_AUTO_DRY;
      remote_state_high |= PHILCO_FAN_AUTO;
      fan_state2 = PHILCO_FAN_AUTO;

    } else {
      switch (this->fan_mode.value()) {
        case climate::CLIMATE_FAN_HIGH:
          remote_state_high |= PHILCO_FAN_HIGH;
          fan_state2 = PHILCO_FAN_HIGH;
          break;
        case climate::CLIMATE_FAN_MEDIUM:
          remote_state_high |= PHILCO_FAN_MED;
          fan_state2 = PHILCO_FAN_MED;
          break;
        case climate::CLIMATE_FAN_LOW:
          remote_state_high |= PHILCO_FAN_LOW;
          fan_state2 = PHILCO_FAN_LOW;
          break;
        case climate::CLIMATE_FAN_AUTO:
        default:
          remote_state_high |= PHILCO_FAN_AUTO;
          fan_state2 = PHILCO_FAN_AUTO;
          break;
      }
    }
  }
  else {
    remote_state_low |= PHILCO_OFF_TEMP;
    remote_state_high |= PHILCO_COOL;
    ESP_LOGE(TAG, "Off Temp code_low: 0x%08" PRIX32, remote_state_low);
  }
  ESP_LOGE(TAG, "Sending philco code_low: 0x%08" PRIX32, remote_state_low);
  ESP_LOGE(TAG, "Sending philco code_high: 0x%08" PRIX32, remote_state_high);
  remote_state.data = remote_state_high;
  remote_state.data = remote_state.data << 32;
  remote_state.data = remote_state.data | remote_state_low;
  ESP_LOGE(TAG, "Fan state: 0x%016" PRIX64, remote_state.data);
  remote_state.data2 = remote_state.data2 + ((uint64_t)((swing_state2) + (fan_state2)) << 48);
  remote_state.nbits = 120;

  ESP_LOGE(TAG, "Sending philco code LSB: 0x%016" PRIX64, remote_state.data);
    ESP_LOGE(TAG, "Sending philco code MSB: 0x%016" PRIX64, remote_state.data2);
  this->transmit_<remote_base::PhilcoProtocol>(remote_state);
}

/*bool PhilcoClimate::on_philco(climate::Climate *parent, remote_base::RemoteReceiveData data) {
  auto decoded = remote_base::PhilcoProtocol().decode(data);
  if (!decoded.has_value())
    return false;
  // Decoded remote state y 3 bytes long code.
  uint32_t remote_state = (*decoded).second;
  ESP_LOGV(TAG, "Decoded 0x%06" PRIX32, remote_state);
  if ((remote_state & 0xFF0000) != 0xB20000)
    return false;

  if (remote_state == COOLIX_OFF) {
    parent->mode = climate::CLIMATE_MODE_OFF;
  } else if (remote_state == COOLIX_SWING) {
    parent->swing_mode =
        parent->swing_mode == climate::CLIMATE_SWING_OFF ? climate::CLIMATE_SWING_VERTICAL : climate::CLIMATE_SWING_OFF;
  } else {
    if ((remote_state & COOLIX_MODE_MASK) == COOLIX_HEAT) {
      parent->mode = climate::CLIMATE_MODE_HEAT;
    } else if ((remote_state & COOLIX_MODE_MASK) == COOLIX_AUTO) {
      parent->mode = climate::CLIMATE_MODE_HEAT_COOL;
    } else if ((remote_state & COOLIX_MODE_MASK) == COOLIX_DRY_FAN) {
      if ((remote_state & COOLIX_FAN_MASK) == COOLIX_FAN_MODE_AUTO_DRY) {
        parent->mode = climate::CLIMATE_MODE_DRY;
      } else {
        parent->mode = climate::CLIMATE_MODE_FAN_ONLY;
      }
    } else
      parent->mode = climate::CLIMATE_MODE_COOL;

    // Fan Speed
    if ((remote_state & COOLIX_FAN_AUTO) == COOLIX_FAN_AUTO || parent->mode == climate::CLIMATE_MODE_HEAT_COOL ||
        parent->mode == climate::CLIMATE_MODE_DRY) {
      parent->fan_mode = climate::CLIMATE_FAN_AUTO;
    } else if ((remote_state & COOLIX_FAN_MIN) == COOLIX_FAN_MIN) {
      parent->fan_mode = climate::CLIMATE_FAN_LOW;
    } else if ((remote_state & COOLIX_FAN_MED) == COOLIX_FAN_MED) {
      parent->fan_mode = climate::CLIMATE_FAN_MEDIUM;
    } else if ((remote_state & COOLIX_FAN_MAX) == COOLIX_FAN_MAX) {
      parent->fan_mode = climate::CLIMATE_FAN_HIGH;
    }

    // Temperature
    uint8_t temperature_code = remote_state & COOLIX_TEMP_MASK;
    for (uint8_t i = 0; i < COOLIX_TEMP_RANGE; i++) {
      if (COOLIX_TEMP_MAP[i] == temperature_code)
        parent->target_temperature = i + COOLIX_TEMP_MIN;
    }
  }
  parent->publish_state();

  return true;
}*/

}  // namespace coolix
}  // namespace esphome
