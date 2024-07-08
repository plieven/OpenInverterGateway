#pragma once

#include "Arduino.h"

#include "Growatt.h"
#include "GrowattTypes.h"

// Growatt modbus protocol version 1.24 from 2020-08-04
typedef enum {
  P124_I_STATUS = 0,
  P124_INPUT_POWER,
  P124_PV1_VOLTAGE,
  P124_PV1_CURRENT,
  P124_PV1_POWER,
  P124_PV2_VOLTAGE,
  P124_PV2_CURRENT,
  P124_PV2_POWER,
  P124_PAC,
  P124_FAC,
  P124_VAC1,
  P124_IAC1,
  P124_PAC1,
  P124_VAC2,
  P124_IAC2,
  P124_PAC2,
  P124_VAC3,
  P124_IAC3,
  P124_PAC3,
  P124_EAC_TODAY,
  P124_EAC_TOTAL,
  P124_TIME_TOTAL,
  P124_EPV1_TODAY,
  P124_EPV1_TOTAL,
  P124_EPV2_TODAY,
  P124_EPV2_TOTAL,
  P124_EPV_TOTAL,
  P124_TEMP1,
  P124_TEMP2,
  P124_TEMP3,
  P124_PDISCHARGE,
  P124_PCHARGE,
  P124_VBAT,
  P124_SOC,
  P124_PAC_TO_USER,
  P124_PAC_TO_USER_TOTAL,
  P124_PAC_TO_GRID,
  P124_PAC_TO_GRID_TOTAL,
  P124_PLOCAL_LOAD,
  P124_PLOCAL_LOAD_TOTAL,
  P124_BATTERY_TEMPERATURE,
  P124_BATTERY_STATE,
  P124_ETOUSER_TODAY,
  P124_ETOUSER_TOTAL,
  P124_ETOGRID_TODAY,
  P124_ETOGRID_TOTAL,
  P124_EDISCHARGE_TODAY,
  P124_EDISCHARGE_TOTAL,
  P124_ECHARGE_TODAY,
  P124_ECHARGE_TOTAL,
  P124_ETOLOCALLOAD_TODAY,
  P124_ETOLOCALLOAD_TOTAL,
  P124_ACCHARGE_TODAY,
  P124_ACCHARGE_TOTAL,
  P124_INPUT_REGISTER_COUNT
} eP124InputRegisters_t;

typedef enum { P124_Active_P_Rate } eP124HoldingRegisters_t;

void init_growatt124(sProtocolDefinition_t &Protocol, Growatt &inverter);

std::tuple<bool, String> getDateTime(const JsonDocument& req, JsonDocument& res,
                                     Growatt& inverter);
std::tuple<bool, String> updateDateTime(const JsonDocument& req,
                                        JsonDocument& res, Growatt& inverter);
std::tuple<bool, String> getPowerActiveRate(const JsonDocument& req,
                                            JsonDocument& res,
                                            Growatt& inverter);
std::tuple<bool, String> setPowerActiveRate(const JsonDocument& req,
                                            JsonDocument& res,
                                            Growatt& inverter);
