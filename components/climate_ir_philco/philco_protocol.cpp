#include "philco_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.philco";

static const uint32_t HEADER_HIGH_US = 8500;
static const uint32_t HEADER_LOW_US = 4200;
static const uint32_t BIT_HIGH_US = 570;
static const uint32_t BIT_ONE_LOW_US = 1600;
static const uint32_t BIT_ZERO_LOW_US = 580;

void PhilcoProtocol::encode(RemoteTransmitData *dst, const PhilcoData &data) {
  dst->set_carrier_frequency(38000); // Confirmed, Philco uses 38000
  dst->reserve(2 + data.nbits * 2u + 3);

  dst->item(HEADER_HIGH_US, HEADER_LOW_US);
  //for (uint64_t mask = 1ULL << (data.nbits - 1); mask != 0; mask >>= 1) {
  for (uint64_t mask = 1ULL; mask != 0; mask <<= 1) {
    if (data.data & mask) {
      //ESP_LOGE(TAG, "adding bit: 1");
      dst->item(BIT_HIGH_US, BIT_ONE_LOW_US);
    } else {
      //ESP_LOGE(TAG, "adding bit: 0");
      dst->item(BIT_HIGH_US, BIT_ZERO_LOW_US);
    }
  }
  uint64_t mask = 1ULL;
  for (uint8_t i = 0; i < data.nbits-64; i++) {
    //ESP_LOGE(TAG, "adding bit2 %d %d" , i, data.data2 & mask > 0 ? 1 : 0);
    if(data.data2 & mask) {
      //ESP_LOGE(TAG, "adding bit2 %d: 1", i);
      dst->item(BIT_HIGH_US, BIT_ONE_LOW_US);
    } else {
      //ESP_LOGE(TAG, "adding bit2 %d: 0", i);
      dst->item(BIT_HIGH_US, BIT_ZERO_LOW_US);
    }
    mask <<= 1;
  }

  // pad message with 3 0 bits
  /*for (uint8_t i = 0; i < 3; i++) {
    ESP_LOGE(TAG, "adding pad bit: 0");
    dst->item(BIT_HIGH_US, BIT_ZERO_LOW_US);
  }*/

  dst->mark(BIT_HIGH_US);
}
optional<PhilcoData> PhilcoProtocol::decode(RemoteReceiveData src) {
  PhilcoData out{
      .data = 0,
      .nbits = 0,
  };
  if (!src.expect_item(HEADER_HIGH_US, HEADER_LOW_US))
    return {};

  for (out.nbits = 0; out.nbits < 32; out.nbits++) {
    if (src.expect_item(BIT_HIGH_US, BIT_ONE_LOW_US)) {
      out.data = (out.data << 1) | 1;
    } else if (src.expect_item(BIT_HIGH_US, BIT_ZERO_LOW_US)) {
      out.data = (out.data << 1) | 0;
    } else if (out.nbits == 28) {
      return out;
    } else {
      return {};
    }
  }

  return out;
}
void PhilcoProtocol::dump(const PhilcoData &data) {
  ESP_LOGI(TAG, "Received Philco: data=0x%08" PRIX32 ", nbits=%d", data.data, data.nbits);
}

}  // namespace remote_base
}  // namespace esphome
