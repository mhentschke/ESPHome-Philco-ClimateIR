#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

#include <cinttypes>

namespace esphome {
namespace philco {

// Temperature
const uint8_t PHILCO_TEMP_MIN = 16;  // Celsius
const uint8_t PHILCO_TEMP_MAX = 32;  // Celsius

class PhilcoClimate : public climate_ir::ClimateIR {
 public:
  PhilcoClimate()
      : climate_ir::ClimateIR(PHILCO_TEMP_MIN, PHILCO_TEMP_MAX, 1.0f, true, true,
                              {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                               climate::CLIMATE_FAN_HIGH},
                              {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL}) {}

  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall &call) override {
    send_swing_cmd_ = call.get_swing_mode().has_value();
    // swing resets after unit powered off
    if (call.get_mode().has_value() && *call.get_mode() == climate::CLIMATE_MODE_OFF)
      this->swing_mode = climate::CLIMATE_SWING_OFF;
    climate_ir::ClimateIR::control(call);
  }

  /// This static method can be used in other climate components that accept the Philco protocol. See midea_ir for
  /// example.
  static bool on_philco(climate::Climate *parent, remote_base::RemoteReceiveData data);

 protected:
  /// Transmit via IR the state of this climate controller.
  void transmit_state() override;
  /// Handle received IR Buffer
  //bool on_receive(remote_base::RemoteReceiveData data) override { return PhilcoClimate::on_philco(this, data); }

  bool send_swing_cmd_{false};
};

}  // namespace philco
}  // namespace esphome
