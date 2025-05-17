#pragma once

#include "esphome/core/component.h"
#include "esphome/components/remote_base/remote_base.h"

#include <cinttypes>

namespace esphome {
namespace remote_base {

struct PhilcoData {
  uint64_t data;
  uint64_t data2;
  uint8_t nbits;

  bool operator==(const PhilcoData &rhs) const { return data == rhs.data && nbits == rhs.nbits; }
};

class PhilcoProtocol : public RemoteProtocol<PhilcoData> {
 public:
  void encode(RemoteTransmitData *dst, const PhilcoData &data) override;
  optional<PhilcoData> decode(RemoteReceiveData src) override;
  void dump(const PhilcoData &data) override;
};

DECLARE_REMOTE_PROTOCOL(Philco)

template<typename... Ts> class PhilcoAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(uint32_t, data)
  TEMPLATABLE_VALUE(uint8_t, nbits)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    PhilcoData data{};
    data.data = this->data_.value(x...);
    data.nbits = this->nbits_.value(x...);
    PhilcoProtocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome
