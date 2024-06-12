#include "Arduino.h"

#include "Config.h"
#include "Growatt.h"
#include "Growatt124.h"
#include <TLog.h>

std::tuple<bool, String> getDateTime(const JsonDocument& req, JsonDocument& res,
                                     Growatt& inverter) {
  String respStr;
  uint16_t year, month, day, hour, minute, second;

#if SIMULATE_INVERTER != 1
  uint16_t result[6];
  bool success = inverter.ReadHoldingRegFrag(45, 6, result);
  if (success) {
    year = result[0];
    month = result[1];
    day = result[2];
    hour = result[3];
    minute = result[4];
    second = result[5];
  }
#else
  year = 2023;
  month = 06;
  day = 22;
  hour = 18;
  minute = 30;
  second = 15;
  bool success = true;
#endif

  if (success) {
    char buf[30];
    snprintf(buf, sizeof(buf), "%04hu-%02hu-%02hu %02hu:%02hu:%02hu", year,
             month, day, hour, minute, second);
    res["value"] = buf;
    return std::make_tuple(true, "Successfully read date/time");
  } else {
    return std::make_tuple(false, "Failed to read date/time");
  }
};

std::tuple<bool, String> updateDateTime(const JsonDocument& req,
                                        JsonDocument& res, Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

  String datetime = req["value"].as<String>();
  if (datetime.length() != 19) {
    return std::make_tuple(false, "Invalid datetime format");
  }

#if SIMULATE_INVERTER != 1
  uint16_t year = datetime.substring(0, 4).toInt();
  uint16_t month = datetime.substring(5, 7).toInt();
  uint16_t day = datetime.substring(8, 10).toInt();
  uint16_t hour = datetime.substring(11, 13).toInt();
  uint16_t minute = datetime.substring(14, 16).toInt();
  uint16_t second = datetime.substring(17, 19).toInt();

  year = year > 2000 ? year - 2000 : 0;

  uint16_t values[] = {year, month, day, hour, minute, second};

  bool success = inverter.WriteHoldingRegFrag(45, 6, values);
#else
  bool success = true;
#endif
  if (success) {
    return std::make_tuple(true, "Successfully updated date/time");
  } else {
    return std::make_tuple(false, "Failed to write date/time");
  }
};

// getLoadFirst, setLoadFirstStopSOC for 3000 mode

std::tuple<String, String> getTimeSlot(uint16_t start, uint16_t stop) {
  int start_hours = (start >> 8) & 0xFF;
  int start_minutes = start & 0xFF;
  int stop_hours = (stop >> 8) & 0xFF;
  int stop_minutes = stop & 0xFF;
  char start_str[8];
  char stop_str[8];
  snprintf(start_str, sizeof(start_str), "%02d:%02d", start_hours,
           start_minutes);
  snprintf(stop_str, sizeof(stop_str), "%02d:%02d", stop_hours, stop_minutes);
  return std::make_tuple(String(start_str), String(stop_str));
}

std::tuple<bool, String> getBatteryFirst(const JsonDocument& req,
                                         JsonDocument& res, Growatt& inverter) {
#if SIMULATE_INVERTER != 1
  uint16_t settings[3];
  if (!inverter.ReadHoldingRegFrag(1090, 3, settings)) {
    return std::make_tuple(false, "Failed to read battery first settings");
  }

  uint16_t powerRate = settings[0];
  uint16_t stopSOC = settings[1];
  bool acChargeEnabled = settings[2] == 1;
#else
  uint16_t powerRate = 100;
  uint16_t stopSOC = 75;
  bool acChargeEnabled = true;
#endif

  res["powerRate"] = powerRate;
  res["stopSOC"] = stopSOC;
  res["acChargeEnabled"] = acChargeEnabled;

#if SIMULATE_INVERTER != 1
  uint16_t timeslots_raw[9];
  if (!inverter.ReadHoldingRegFrag(1100, 9, timeslots_raw)) {
    return std::make_tuple(false, "Failed to read battery first timeslots");
  }
#endif

  auto timeslots = res.createNestedArray("timeSlots");
  for (int i = 0; i < 3; i++) {
    auto timeslot = timeslots.createNestedObject();
    String start_str;
    String stop_str;
    bool enabled;
#if SIMULATE_INVERTER != 1
    std::tie(start_str, stop_str) =
        getTimeSlot(timeslots_raw[i * 3], timeslots_raw[i * 3 + 1]);
    enabled = timeslots_raw[i * 3 + 2] == 1;
#else
    start_str = "01:30";
    stop_str = "05:30";
    enabled = true;
#endif
    timeslot["slot"] = i + 1;
    timeslot["start"] = start_str;
    timeslot["stop"] = stop_str;
    timeslot["enabled"] = enabled;
  }

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setBatteryFirstPowerRate(const JsonDocument& req,
                                                  JsonDocument& res,
                                                  Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1090, value)) {
    return std::make_tuple(false, "Failed to write battery first power rate");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setBatteryFirstStopSOC(const JsonDocument& req,
                                                JsonDocument& res,
                                                Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1091, value)) {
    return std::make_tuple(false, "Failed to write battery first stop SOC");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setBatteryFirstACChargeEnabled(const JsonDocument& req,
                                                        JsonDocument& res,
                                                        Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1092, value)) {
    return std::make_tuple(false,
                           "Failed to write battery first AC charge enabled");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setTimeSlot(const JsonDocument& req, JsonDocument& res,
                                     Growatt& inverter, uint16_t startReg) {
  if (!req.containsKey("start")) {
    return std::make_tuple(false, "'start' field is required");
  }
  String start_str = req["start"].as<String>();

  if (!req.containsKey("stop")) {
    return std::make_tuple(false, "'stop' field is required");
  }
  String stop_str = req["stop"].as<String>();

  if (!req.containsKey("enabled")) {
    return std::make_tuple(false, "'enabled' field is required");
  }

#if SIMULATE_INVERTER != 1
  bool enabled = req["enabled"].as<bool>();
#endif

  if (start_str.length() != 5 || stop_str.length() != 5 ||
      start_str[2] != ':' || stop_str[2] != ':') {
    return std::make_tuple(false, "Invalid time format");
  }

  if (!req.containsKey("slot")) {
    return std::make_tuple(false, "'slot' field is required");
  }
  uint16_t slot = req["slot"].as<uint16_t>();

  if (slot < 1 || slot > 3) {
    return std::make_tuple(false,
                           "Invalid slot number, must be between 1 and 3");
  }

#if SIMULATE_INVERTER != 1
  int start_hours = start_str.substring(0, 2).toInt();
  int start_minutes = start_str.substring(3, 5).toInt();

  int stop_hours = stop_str.substring(0, 2).toInt();
  int stop_minutes = stop_str.substring(3, 5).toInt();

  uint16_t time_start = (start_hours << 8) | start_minutes;
  uint16_t time_stop = (stop_hours << 8) | stop_minutes;

  uint16_t timeslot_raw[3];
  timeslot_raw[0] = time_start;
  timeslot_raw[1] = time_stop;
  timeslot_raw[2] = enabled ? 1 : 0;

  uint16_t timeslot_start_addr = startReg + ((slot - 1) * 3);
  if (!inverter.WriteHoldingRegFrag(timeslot_start_addr, 3, timeslot_raw)) {
    return std::make_tuple(false, "Failed to write timeslot");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setBatteryFirstTimeSlot(const JsonDocument& req,
                                                 JsonDocument& res,
                                                 Growatt& inverter) {
  return setTimeSlot(req, res, inverter, 1100);
}

std::tuple<bool, String> getGridFirst(const JsonDocument& req,
                                      JsonDocument& res, Growatt& inverter) {
#if SIMULATE_INVERTER != 1
  uint16_t settings[3];
  if (!inverter.ReadHoldingRegFrag(1070, 2, settings)) {
    return std::make_tuple(false, "Failed to read grid first settings");
  }

  uint16_t powerRate = settings[0];
  uint16_t stopSOC = settings[1];
#else
  uint16_t powerRate = 100;
  uint16_t stopSOC = 5;
#endif

  res["powerRate"] = powerRate;
  res["stopSOC"] = stopSOC;

#if SIMULATE_INVERTER != 1
  uint16_t timeslots_raw[9];
  if (!inverter.ReadHoldingRegFrag(1080, 9, timeslots_raw)) {
    return std::make_tuple(false, "Failed to read grid first timeslots");
  }
#endif

  auto timeslots = res.createNestedArray("timeSlots");
  for (int i = 0; i < 3; i++) {
    auto timeslot = timeslots.createNestedObject();
    String start_str;
    String stop_str;
    bool enabled;
#if SIMULATE_INVERTER != 1
    std::tie(start_str, stop_str) =
        getTimeSlot(timeslots_raw[i * 3], timeslots_raw[i * 3 + 1]);
    enabled = timeslots_raw[i * 3 + 2] == 1;
#else
    start_str = "01:30";
    stop_str = "05:30";
    enabled = true;
#endif
    timeslot["slot"] = i + 1;
    timeslot["start"] = start_str;
    timeslot["stop"] = stop_str;
    timeslot["enabled"] = enabled;
  }

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setGridFirstPowerRate(const JsonDocument& req,
                                               JsonDocument& res,
                                               Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1070, value)) {
    return std::make_tuple(false, "Failed to write grid first power rate");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setGridFirstStopSOC(const JsonDocument& req,
                                             JsonDocument& res,
                                             Growatt& inverter) {
  if (!req.containsKey("value")) {
    return std::make_tuple(false, "'value' field is required");
  }

#if SIMULATE_INVERTER != 1
  uint16_t value = req["value"].as<uint16_t>();
  if (!inverter.WriteHoldingReg(1071, value)) {
    return std::make_tuple(false, "Failed to write grid first stop SOC");
  }
#endif

  return std::make_tuple(true, "success");
}

std::tuple<bool, String> setGridFirstTimeSlot(const JsonDocument& req,
                                              JsonDocument& res,
                                              Growatt& inverter) {
  return setTimeSlot(req, res, inverter, 1080);
}

#ifndef TEMPERATURE_WORKAROUND_MULTIPLIER
#define TEMPERATURE_WORKAROUND_MULTIPLIER 0.1
#endif

// NOTE: my inverter (SPH4-10KTL3 BH-UP) only manages to read 64 registers in
// one read!
void init_growatt124(sProtocolDefinition_t& Protocol, Growatt& inverter) {
    // definition of input registers
    Protocol.InputRegisterCount = P124_INPUT_REGISTER_COUNT;
    // address, value, size, name, multiplier, unit, frontend, plot
#if ENABLE_VERSION124_IREG3000 == 1
    // FRAGMENT 1: BEGIN
    Protocol.InputRegisters[P124_INVERTER_STATUS] = sGrowattModbusReg_t{
        3000, 0, SIZE_16BIT, F("InverterStatus"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_INVERTER_RUNSTATE] = sGrowattModbusReg_t{
        3000, 0, SIZE_16BIT, F("InverterRunState"), 1, 1, NONE, false, false};
    Protocol.InputRegisters[P124_PPV] = sGrowattModbusReg_t{
        3001, 0, SIZE_32BIT, F("PVTotalPower"), 0.1, 0.1, POWER_W, true, true};
    Protocol.InputRegisters[P124_VPV1] =
    sGrowattModbusReg_t{3003,       0,     SIZE_16BIT, F("PV1Voltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IPV1] = sGrowattModbusReg_t{ //TODO: real reg names
        3004,       0,     SIZE_16BIT, F("PV1InputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PPV1] = sGrowattModbusReg_t{
        3005,       0,     SIZE_32BIT, F("PV1InputPower"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_VPV2] =
    sGrowattModbusReg_t{3007,       0,     SIZE_16BIT, F("PV2Voltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IPV2] = sGrowattModbusReg_t{
        3008,       0,     SIZE_16BIT, F("PV2InputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PPV2] = sGrowattModbusReg_t{
        3009,       0,     SIZE_32BIT, F("PV2InputPower"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_VPV3] =
    sGrowattModbusReg_t{3011,       0,     SIZE_16BIT, F("PV3Voltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IPV3] = sGrowattModbusReg_t{
        3012,       0,     SIZE_16BIT, F("PV3InputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PPV3] = sGrowattModbusReg_t{
        3013,       0,     SIZE_32BIT, F("PV3InputPower"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_VPV4] =
    sGrowattModbusReg_t{3015,       0,     SIZE_16BIT, F("PV4Voltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IPV4] = sGrowattModbusReg_t{
        3016,       0,     SIZE_16BIT, F("PV4InputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PPV4] = sGrowattModbusReg_t{
        3017,       0,     SIZE_32BIT, F("PV4InputPower"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_PSYS] = sGrowattModbusReg_t{
        3019,      0,    SIZE_32BIT_S, F("SystemOutputPower"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_QAC] = sGrowattModbusReg_t{
        3021,      0,    SIZE_32BIT_S, F("ReactivePower"), 0.1, 0.1,
        POWER_W, true, true};
    Protocol.InputRegisters[P124_PAC] = sGrowattModbusReg_t{
        3023,      0,    SIZE_32BIT_S, F("OutputPower"), 0.1, 0.1,
        POWER_W, true, true};
    Protocol.InputRegisters[P124_FAC] = sGrowattModbusReg_t{
        3025,        0,     SIZE_16BIT, F("GridFrequency"), 0.01, 0.01,
        FREQUENCY, false, false};
    Protocol.InputRegisters[P124_VAC1] = sGrowattModbusReg_t{
        3026,      0,     SIZE_16BIT, F("L1ThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IAC1] = sGrowattModbusReg_t{
        3027,      0,     SIZE_16BIT, F("L1ThreePhaseGridOutputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PAC1] = sGrowattModbusReg_t{
        3028, 0,     SIZE_32BIT, F("L1ThreePhaseGridOutputPower"), 0.1, 0.1,
        VA, false, false};
    Protocol.InputRegisters[P124_VAC2] = sGrowattModbusReg_t{
        3030,      0,     SIZE_16BIT, F("L2ThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IAC2] = sGrowattModbusReg_t{
        3031,      0,     SIZE_16BIT, F("L2ThreePhaseGridOutputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PAC2] = sGrowattModbusReg_t{
        3032, 0,     SIZE_32BIT, F("L2ThreePhaseGridOutputPower"), 0.1, 0.1,
        VA, false, false};
    Protocol.InputRegisters[P124_VAC3] = sGrowattModbusReg_t{
        3034,      0,     SIZE_16BIT, F("L3ThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IAC3] = sGrowattModbusReg_t{
        3035,      0,     SIZE_16BIT, F("L3ThreePhaseGridOutputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PAC3] = sGrowattModbusReg_t{
        3036, 0,     SIZE_32BIT, F("L3ThreePhaseGridOutputPower"), 0.1, 0.1,
        VA, false, false};
    Protocol.InputRegisters[P124_VAC_RS] = sGrowattModbusReg_t{
        3038,      0,     SIZE_16BIT, F("RSThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_VAC_ST] = sGrowattModbusReg_t{
        3039,      0,     SIZE_16BIT, F("STThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_VAC_TR] = sGrowattModbusReg_t{
        3040,      0,     SIZE_16BIT, F("TRThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_PTOUSER_TOTAL] = sGrowattModbusReg_t{
        3041,      0,    SIZE_32BIT_S, F("TotalForwardPower"), 0.1, 0.1,
        POWER_W, true, true};
    Protocol.InputRegisters[P124_PTOGRID_TOTAL] = sGrowattModbusReg_t{
        3043,      0,    SIZE_32BIT_S, F("TotalReversePower"), 0.1, 0.1,
        POWER_W, true, true};
    Protocol.InputRegisters[P124_PTOLOAD_TOTAL] = sGrowattModbusReg_t{
        3045,      0,    SIZE_32BIT_S, F("TotalLoadPower"), 0.1, 0.1,
        POWER_W, true, true};
    Protocol.InputRegisters[P124_TIME_TOTAL] = sGrowattModbusReg_t{
        3047,      0,     SIZE_32BIT, F("WorkTimeTotal"), 0.5, 1,
        SECONDS, false, false};
    Protocol.InputRegisters[P124_EAC_TODAY] = sGrowattModbusReg_t{
        3049,        0,    SIZE_32BIT, F("TodayGenerateEnergy"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_EAC_TOTAL] = sGrowattModbusReg_t{
        3051,        0,    SIZE_32BIT, F("TotalGenerateEnergy"), 0.1, 0.1,
        POWER_KWH, true, false};
      Protocol.InputRegisters[P124_EPV_TOTAL] = sGrowattModbusReg_t{
          3053,        0,     SIZE_32BIT, F("PVEnergyTotal"), 0.1, 0.1,
          POWER_KWH, false, false};
      Protocol.InputRegisters[P124_EPV1_TODAY] = sGrowattModbusReg_t{
          3055,        0,     SIZE_32BIT, F("PV1EnergyToday"), 0.1, 0.1,
          POWER_KWH, false, false};
      Protocol.InputRegisters[P124_EPV1_TOTAL] = sGrowattModbusReg_t{
          3057,        0,     SIZE_32BIT, F("PV1EnergyTotal"), 0.1, 0.1,
          POWER_KWH, false, false};
      Protocol.InputRegisters[P124_EPV2_TODAY] = sGrowattModbusReg_t{
          3059,        0,     SIZE_32BIT, F("PV2EnergyToday"), 0.1, 0.1,
          POWER_KWH, false, false};
      Protocol.InputRegisters[P124_EPV2_TOTAL] = sGrowattModbusReg_t{
          3061,        0,     SIZE_32BIT, F("PV2EnergyTotal"), 0.1, 0.1,
          POWER_KWH, false, false};
    // FRAGMENT 1: END

    // FRAGMENT 2: BEGIN
    Protocol.InputRegisters[P124_EPV3_TODAY] = sGrowattModbusReg_t{
        3063,        0,     SIZE_32BIT, F("PV3EnergyToday"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_EPV3_TOTAL] = sGrowattModbusReg_t{
        3065,        0,     SIZE_32BIT, F("PV3EnergyTotal"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_ETOUSER_TODAY] = sGrowattModbusReg_t{
        3067,      0,    SIZE_32BIT, F("TodayEnergyToUser"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ETOUSER_TOTAL] = sGrowattModbusReg_t{
        3069,      0,    SIZE_32BIT, F("TotalEnergyToUser"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ETOGRID_TODAY] = sGrowattModbusReg_t{
        3071,      0,    SIZE_32BIT, F("TodayEnergyToGrid"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ETOGRID_TOTAL] = sGrowattModbusReg_t{
        3073,      0,    SIZE_32BIT, F("TotalEnergyToGrid"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ELOAD_TODAY] = sGrowattModbusReg_t{
        3075,      0,    SIZE_32BIT, F("TodayEnergyOfUserLoad"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ELOAD_TOTAL] = sGrowattModbusReg_t{
        3077,      0,    SIZE_32BIT, F("TotalEnergyOfUserLoad"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_EPV4_TODAY] = sGrowattModbusReg_t{
        3079,        0,     SIZE_32BIT, F("PV4EnergyToday"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_EPV4_TOTAL] = sGrowattModbusReg_t{
        3081,        0,     SIZE_32BIT, F("PV4EnergyTotal"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_EPV_TODAY] = sGrowattModbusReg_t{
        3083,        0,     SIZE_32BIT, F("PVEnergyToday"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_DERATING_MODE] = sGrowattModbusReg_t{
        3086,          0,     SIZE_16BIT, F("DeratingMode"), 1, 1,
        NONE, true, false};   //TODO: decode
    Protocol.InputRegisters[P124_ISO] = sGrowattModbusReg_t{
        3087,          0,     SIZE_16BIT, F("PVISOValue"), 1, 1,
        RESISTANCE_K, false, false};
    Protocol.InputRegisters[P124_DCI_R] = sGrowattModbusReg_t{
        3088,          0,     SIZE_16BIT, F("RDCICurr"), 0.1, 0.1,
        CURRENT_M, false, false};
    Protocol.InputRegisters[P124_DCI_S] = sGrowattModbusReg_t{
        3089,          0,     SIZE_16BIT, F("SDCICurr"), 0.1, 0.1,
        CURRENT_M, false, false};
    Protocol.InputRegisters[P124_DCI_T] = sGrowattModbusReg_t{
        3090,          0,     SIZE_16BIT, F("TDCICurr"), 0.1, 0.1,
        CURRENT_M, false, false};
    Protocol.InputRegisters[P124_GFCI] = sGrowattModbusReg_t{
        3091,          0,     SIZE_16BIT, F("GFCICurr"), 1, 1,
        CURRENT_M, false, false};
    Protocol.InputRegisters[P124_BUS_VOLTAGE] = sGrowattModbusReg_t{
        3092,      0,     SIZE_16BIT, F("TotalBusVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
      Protocol.InputRegisters[P124_TEMP1] = sGrowattModbusReg_t{
          3093,          0,    SIZE_16BIT, F("InverterTemperature"), 0.1, 0.1,
          TEMPERATURE, true, false};
      Protocol.InputRegisters[P124_TEMP2] = sGrowattModbusReg_t{
          3094,          0,     SIZE_16BIT, F("TemperatureInsideIPM"), 0.1, 0.1,
          TEMPERATURE, false, false};
      Protocol.InputRegisters[P124_TEMP3] = sGrowattModbusReg_t{
          3095,          0,     SIZE_16BIT, F("BoostTemperature"), 0.1, 0.1,
          TEMPERATURE, false, false};
    Protocol.InputRegisters[P124_TEMP5] = sGrowattModbusReg_t{
        3097,          0,     SIZE_16BIT, F("CommunicationBoardTemperature"), 0.1, 0.1,
        TEMPERATURE, false, false};
    Protocol.InputRegisters[P124_P_BUS_VOLTAGE] = sGrowattModbusReg_t{
        3098,      0,     SIZE_16BIT, F("PBusInsideVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_N_BUS_VOLTAGE] = sGrowattModbusReg_t{
        3099,      0,     SIZE_16BIT, F("NBusInsideVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IPF] = sGrowattModbusReg_t{
        3100,      0,     SIZE_16BIT, F("InverterOutputPFNow"), 1, 1,
        NONE, false, false};
    Protocol.InputRegisters[P124_REALOPPERCENT] = sGrowattModbusReg_t{
        3101, 0, SIZE_16BIT, F("RealOutputPercent"), 1, 1, PERCENTAGE, true, false};
    Protocol.InputRegisters[P124_OPFULLWATT] = sGrowattModbusReg_t{
        3102, 0, SIZE_32BIT, F("OutputMaxpowerLimited"), 0.1, 0.1, POWER_W, true, true};
      Protocol.InputRegisters[P124_FAULT_MAINCODE] = sGrowattModbusReg_t{
          3105, 0, SIZE_16BIT, F("InverterFaultMaincode"), 1, 1, NONE, true, false};
      Protocol.InputRegisters[P124_WARN_MAINCODE] = sGrowattModbusReg_t{
          3106, 0, SIZE_16BIT, F("InverterWarnMaincode"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_AFCI_STATUS] = sGrowattModbusReg_t{
        3112, 0, SIZE_16BIT, F("AFCIStatus"), 1, 1, NONE, false, false};
    Protocol.InputRegisters[P124_INV_START_DELAY] = sGrowattModbusReg_t{
        3115, 0, SIZE_16BIT, F("InvStartDelayTime"), 1, 1, SECONDS, false, false};
    Protocol.InputRegisters[P124_BDC_ONOFFSTATE] = sGrowattModbusReg_t{
        3118, 0, SIZE_16BIT, F("BDCConnectState"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_DRYCONTACTSTATE] = sGrowattModbusReg_t{
        3119, 0, SIZE_16BIT, F("DryContactState"), 1, 1, NONE, false, false};
    Protocol.InputRegisters[P124_PSELF] = sGrowattModbusReg_t{
        3121, 0, SIZE_32BIT, F("SelfUsePower"), 0.1, 0.1, POWER_W, false, false};
    Protocol.InputRegisters[P124_ESYS_TODAY] = sGrowattModbusReg_t{
        3123, 0, SIZE_32BIT, F("SystemEnergyToday"), 0.1, 0.1, POWER_KWH, false, false};
    // FRAGMENT 2: END

    // FRAGMENT 3: BEGIN
    Protocol.InputRegisters[P124_EDISCHR_TODAY] = sGrowattModbusReg_t{
        3125, 0, SIZE_32BIT, F("DischargeEnergyToday"), 0.1, 0.1, POWER_KWH, true, false};
    Protocol.InputRegisters[P124_EDISCHR_TOTAL] = sGrowattModbusReg_t{
        3127, 0, SIZE_32BIT, F("DischargeEnergyTotal"), 0.1, 0.1, POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ECHR_TODAY] = sGrowattModbusReg_t{
        3129, 0, SIZE_32BIT, F("ChargeEnergyToday"), 0.1, 0.1, POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ECHR_TOTAL] = sGrowattModbusReg_t{
        3131, 0, SIZE_32BIT, F("ChargeEnergyTotal"), 0.1, 0.1, POWER_KWH, true, false};
    Protocol.InputRegisters[P124_EACCHR_TODAY] = sGrowattModbusReg_t{
        3133, 0, SIZE_32BIT, F("ACChargeEnergyToday"), 0.1, 0.1, POWER_KWH, true, false};
    Protocol.InputRegisters[P124_EACCHR_TOTAL] = sGrowattModbusReg_t{
        3135, 0, SIZE_32BIT, F("ACChargeEnergyTotal"), 0.1, 0.1, POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ESYS_TOTAL] = sGrowattModbusReg_t{
        3137, 0, SIZE_32BIT, F("SystemEnergyTotal"), 0.1, 0.1, POWER_KWH, false, false};
    Protocol.InputRegisters[P124_ESELF_TODAY] = sGrowattModbusReg_t{
        3139, 0, SIZE_32BIT, F("SelfOutputEnergyToday"), 0.1, 0.1, POWER_KWH, false, false};
    Protocol.InputRegisters[P124_ESELF_TOTAL] = sGrowattModbusReg_t{
        3141, 0, SIZE_32BIT, F("SelfOutputEnergyTotal"), 0.1, 0.1, POWER_KWH, false, false};
    Protocol.InputRegisters[P124_PRIORITY] = sGrowattModbusReg_t{
        3144, 0, SIZE_16BIT, F("Priority"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_BDC_DERATINGMODE] = sGrowattModbusReg_t{
        3165, 0, SIZE_16BIT, F("BDCDeratingMode"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_BDC_SYSSTATE] = sGrowattModbusReg_t{
        3166, 0, SIZE_16BIT, F("BDCSysState"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_BDC_SYSMODE] = sGrowattModbusReg_t{
        3166, 0, SIZE_16BIT, F("BDCSysMode"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_BDC_FAULTCODE] = sGrowattModbusReg_t{
        3167, 0, SIZE_16BIT, F("BDCFaultCode"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_BDC_WARNCODE] = sGrowattModbusReg_t{
        3168, 0, SIZE_16BIT, F("BDCWarnCode"), 1, 1, NONE, true, false};
      Protocol.InputRegisters[P124_BDC_VBAT] = sGrowattModbusReg_t{
          3169,    0,     SIZE_16BIT, F("BDCBatteryVoltage"), 0.1, 0.1,
          VOLTAGE, true, false};
    Protocol.InputRegisters[P124_BDC_IBAT] = sGrowattModbusReg_t{
        3170,       0,     SIZE_16BIT, F("BDCBatteryCurrent"), 0.1, 0.1,
        CURRENT, true, false};
      Protocol.InputRegisters[P124_BDC_SOC] = sGrowattModbusReg_t{
          3171, 0, SIZE_16BIT, F("BDCStateOfCharge"), 1, 1, PERCENTAGE, true, false};
    Protocol.InputRegisters[P124_BDC_VBUS1] = sGrowattModbusReg_t{
        3172,       0,     SIZE_16BIT, F("BDCTotalBusVoltage"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_BDC_VBUS2] = sGrowattModbusReg_t{
        3173,       0,     SIZE_16BIT, F("BDCOnTheBusVoltage"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_BDC_IBB] = sGrowattModbusReg_t{
        3174,       0,     SIZE_16BIT, F("BDCBuckBoostCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_BDC_ILLC] = sGrowattModbusReg_t{
        3175,       0,     SIZE_16BIT, F("BDCLlcCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_BDC_TEMPA] = sGrowattModbusReg_t{
        3176,          0,    SIZE_16BIT, F("BDCTemperatureA"), 0.1, 0.1,
        TEMPERATURE, true, false};
    Protocol.InputRegisters[P124_BDC_TEMPB] = sGrowattModbusReg_t{
        3177,          0,     SIZE_16BIT, F("BDCTemperatureB"), 0.1, 0.1,
        TEMPERATURE, false, false};
      Protocol.InputRegisters[P124_BDC_PDISCHR] = sGrowattModbusReg_t{
          3178,    0,    SIZE_32BIT, F("BDCDischargePower"), 0.1, 0.1,
          POWER_W, true, true};
      Protocol.InputRegisters[P124_BDC_PCHR] =
          sGrowattModbusReg_t{3180,    0,    SIZE_32BIT, F("BDCChargePower"), 0.1, 0.1,
                              POWER_W, true, true};
    Protocol.InputRegisters[P124_BDC_EDISCHR_TOTAL] = sGrowattModbusReg_t{
        3182, 0, SIZE_32BIT, F("BDCDischargeEnergyTotal"), 0.1, 0.1, POWER_KWH, false, false};
    Protocol.InputRegisters[P124_BDC_ECHR_TOTAL] = sGrowattModbusReg_t{
        3184, 0, SIZE_32BIT, F("BDCChargeEnergyTotal"), 0.1, 0.1, POWER_KWH, false, false};
    // FRAGMENT 3: END
    
    Protocol.InputReadFragments[Protocol.InputFragmentCount++] = sGrowattReadFragment_t{3000, 63};
    Protocol.InputReadFragments[Protocol.InputFragmentCount++] = sGrowattReadFragment_t{3063, 62};
    Protocol.InputReadFragments[Protocol.InputFragmentCount++] = sGrowattReadFragment_t{3125, 61};
#else
    // FRAGMENT 1: BEGIN
    Protocol.InputRegisters[P124_I_STATUS] = sGrowattModbusReg_t{
        0, 0, SIZE_16BIT, F("InverterStatus"), 1, 1, NONE, true, false};
    Protocol.InputRegisters[P124_INPUT_POWER] = sGrowattModbusReg_t{
        1, 0, SIZE_32BIT, F("InputPower"), 0.1, 0.1, POWER_W, true, true};
    
    Protocol.InputRegisters[P124_PV1_VOLTAGE] =
    sGrowattModbusReg_t{3,       0,     SIZE_16BIT, F("PV1Voltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_PV1_CURRENT] = sGrowattModbusReg_t{
        4,       0,     SIZE_16BIT, F("PV1InputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PV1_POWER] = sGrowattModbusReg_t{
        5,       0,     SIZE_32BIT, F("PV1InputPower"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_PV2_VOLTAGE] =
    sGrowattModbusReg_t{7,       0,     SIZE_16BIT, F("PV2Voltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_PV2_CURRENT] = sGrowattModbusReg_t{
        8,       0,     SIZE_16BIT, F("PV2InputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PV2_POWER] = sGrowattModbusReg_t{
        9,       0,     SIZE_32BIT, F("PV2InputPower"), 0.1, 0.1,
        POWER_W, false, false};
    
    Protocol.InputRegisters[P124_PAC] = sGrowattModbusReg_t{
        35,      0,    SIZE_32BIT_S, F("OutputPower"), 0.1, 0.1,
        POWER_W, true, true};
    Protocol.InputRegisters[P124_FAC] = sGrowattModbusReg_t{
        37,        0,     SIZE_16BIT, F("GridFrequency"), 0.01, 0.01,
        FREQUENCY, false, false};
    
    Protocol.InputRegisters[P124_VAC1] = sGrowattModbusReg_t{
        38,      0,     SIZE_16BIT, F("L1ThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IAC1] = sGrowattModbusReg_t{
        39,      0,     SIZE_16BIT, F("L1ThreePhaseGridOutputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PAC1] = sGrowattModbusReg_t{
        40, 0,     SIZE_32BIT, F("L1ThreePhaseGridOutputPower"), 0.1, 0.1,
        VA, false, false};
    Protocol.InputRegisters[P124_VAC2] = sGrowattModbusReg_t{
        42,      0,     SIZE_16BIT, F("L2ThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IAC2] = sGrowattModbusReg_t{
        43,      0,     SIZE_16BIT, F("L2ThreePhaseGridOutputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PAC2] = sGrowattModbusReg_t{
        44, 0,     SIZE_32BIT, F("L2ThreePhaseGridOutputPower"), 0.1, 0.1,
        VA, false, false};
    Protocol.InputRegisters[P124_VAC3] = sGrowattModbusReg_t{
        46,      0,     SIZE_16BIT, F("L3ThreePhaseGridVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_IAC3] = sGrowattModbusReg_t{
        47,      0,     SIZE_16BIT, F("L3ThreePhaseGridOutputCurrent"), 0.1, 0.1,
        CURRENT, false, false};
    Protocol.InputRegisters[P124_PAC3] = sGrowattModbusReg_t{
        48, 0,     SIZE_32BIT, F("L3ThreePhaseGridOutputPower"), 0.1, 0.1,
        VA, false, false};
    // FRAGMENT 1: END
    
    // FRAGMENT 2: BEGIN
    Protocol.InputRegisters[P124_EAC_TODAY] = sGrowattModbusReg_t{
        53,        0,    SIZE_32BIT, F("TodayGenerateEnergy"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_EAC_TOTAL] = sGrowattModbusReg_t{
        55,        0,    SIZE_32BIT, F("TotalGenerateEnergy"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_TIME_TOTAL] = sGrowattModbusReg_t{
        57,      0,     SIZE_32BIT, F("TWorkTimeTotal"), 0.5, 1,
        SECONDS, false, false};
    
    Protocol.InputRegisters[P124_EPV1_TODAY] = sGrowattModbusReg_t{
        59,        0,     SIZE_32BIT, F("PV1EnergyToday"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_EPV1_TOTAL] = sGrowattModbusReg_t{
        61,        0,     SIZE_32BIT, F("PV1EnergyTotal"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_EPV2_TODAY] = sGrowattModbusReg_t{
        63,        0,     SIZE_32BIT, F("PV2EnergyToday"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_EPV2_TOTAL] = sGrowattModbusReg_t{
        65,        0,     SIZE_32BIT, F("PV2EnergyTotal"), 0.1, 0.1,
        POWER_KWH, false, false};
    Protocol.InputRegisters[P124_EPV_TOTAL] = sGrowattModbusReg_t{
        91,        0,     SIZE_32BIT, F("PVEnergyTotal"), 0.1, 0.1,
        POWER_KWH, false, false};
    
    Protocol.InputRegisters[P124_TEMP1] = sGrowattModbusReg_t{
        93,          0,    SIZE_16BIT, F("InverterTemperature"), 0.1, 0.1,
        TEMPERATURE, true, true};
    Protocol.InputRegisters[P124_TEMP2] = sGrowattModbusReg_t{
        94,          0,     SIZE_16BIT, F("TemperatureInsideIPM"), 0.1, 0.1,
        TEMPERATURE, false, false};
    Protocol.InputRegisters[P124_TEMP3] = sGrowattModbusReg_t{
        95,          0,     SIZE_16BIT, F("BoostTemperature"), 0.1, 0.1,
        TEMPERATURE, false, false};
    // FRAGMENT 2: END
    
    // FRAGMENT 3: BEGIN
    Protocol.InputRegisters[P124_PDISCHARGE] = sGrowattModbusReg_t{
        1009,    0,    SIZE_32BIT, F("DischargePower"), 0.1, 0.1,
        POWER_W, true, true};
    Protocol.InputRegisters[P124_PCHARGE] =
    sGrowattModbusReg_t{1011,    0,    SIZE_32BIT, F("ChargePower"), 0.1, 0.1,
        POWER_W, true, true};
    Protocol.InputRegisters[P124_VBAT] = sGrowattModbusReg_t{
        1013,    0,     SIZE_16BIT, F("BatteryVoltage"), 0.1, 0.1,
        VOLTAGE, false, false};
    Protocol.InputRegisters[P124_SOC] = sGrowattModbusReg_t{
        1014, 0, SIZE_16BIT, F("SOC"), 1, 1, PERCENTAGE, true, true};
    Protocol.InputRegisters[P124_PAC_TO_USER] = sGrowattModbusReg_t{
        1015,    0,     SIZE_32BIT, F("ACPowerToUser"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_PAC_TO_USER_TOTAL] = sGrowattModbusReg_t{
        1021,    0,     SIZE_32BIT, F("ACPowerToUserTotal"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_PAC_TO_GRID] = sGrowattModbusReg_t{
        1023,    0,     SIZE_32BIT, F("ACPowerToGrid"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_PAC_TO_GRID_TOTAL] = sGrowattModbusReg_t{
        1029,    0,     SIZE_32BIT, F("ACPowerToGridTotal"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_PLOCAL_LOAD] = sGrowattModbusReg_t{
        1031,    0,     SIZE_32BIT, F("INVPowerToLocalLoad"), 0.1, 0.1,
        POWER_W, false, false};
    Protocol.InputRegisters[P124_PLOCAL_LOAD_TOTAL] = sGrowattModbusReg_t{
        1037,    0,    SIZE_32BIT, F("INVPowerToLocalLoadTotal"), 0.1, 0.1,
        POWER_W, true, false};
    Protocol.InputRegisters[P124_BATTERY_TEMPERATURE] =
    sGrowattModbusReg_t{1040,
        0,
        SIZE_16BIT,
        F("BatteryTemperature"),
        TEMPERATURE_WORKAROUND_MULTIPLIER,
        TEMPERATURE_WORKAROUND_MULTIPLIER,
        TEMPERATURE,
        true,
        true};
    Protocol.InputRegisters[P124_BATTERY_STATE] = sGrowattModbusReg_t{
        1041, 0, SIZE_16BIT, F("BatteryState"), 1, 1, NONE, true, false};
    
    Protocol.InputRegisters[P124_ETOUSER_TODAY] = sGrowattModbusReg_t{
        1044,      0,    SIZE_32BIT, F("EnergyToUserToday"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ETOUSER_TOTAL] = sGrowattModbusReg_t{
        1046,      0,    SIZE_32BIT, F("EnergyToUserTotal"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ETOGRID_TODAY] = sGrowattModbusReg_t{
        1048,      0,    SIZE_32BIT, F("EnergyToGridToday"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ETOGRID_TOTAL] = sGrowattModbusReg_t{
        1050,      0,    SIZE_32BIT, F("EnergyToGridTotal"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_EDISCHARGE_TODAY] = sGrowattModbusReg_t{
        1052,      0,    SIZE_32BIT, F("DischargeEnergyToday"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_EDISCHARGE_TOTAL] = sGrowattModbusReg_t{
        1054,      0,    SIZE_32BIT, F("DischargeEnergyTotal"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ECHARGE_TODAY] = sGrowattModbusReg_t{
        1056,      0,    SIZE_32BIT, F("ChargeEnergyToday"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ECHARGE_TOTAL] = sGrowattModbusReg_t{
        1058,      0,    SIZE_32BIT, F("ChargeEnergyTotal"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ETOLOCALLOAD_TODAY] = sGrowattModbusReg_t{
        1060,      0,    SIZE_32BIT, F("LocalLoadEnergyToday"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ETOLOCALLOAD_TOTAL] = sGrowattModbusReg_t{
        1062,      0,    SIZE_32BIT, F("LocalLoadEnergyTotal"), 0.1, 0.1,
        POWER_KWH, true, false};
    // FRAGMENT 3: END
    
    // FRAGMENT 4: START
    Protocol.InputRegisters[P124_ACCHARGE_TODAY] = sGrowattModbusReg_t{
        1124,      0,    SIZE_32BIT, F("ACChargeEnergyToday"), 0.1, 0.1,
        POWER_KWH, true, false};
    Protocol.InputRegisters[P124_ACCHARGE_TOTAL] = sGrowattModbusReg_t{
        1126,      0,    SIZE_32BIT, F("ACChargeEnergyTotal"), 0.1, 0.1,
        POWER_KWH, true, false};
    // FRAGMENT 4: END
    
    Protocol.InputFragmentCount = 4;
    Protocol.InputReadFragments[0] = sGrowattReadFragment_t{0, 50};
    Protocol.InputReadFragments[1] = sGrowattReadFragment_t{53, 59};
    Protocol.InputReadFragments[2] = sGrowattReadFragment_t{1009, 55};
    Protocol.InputReadFragments[3] = sGrowattReadFragment_t{1124, 4};
#endif
    
    Protocol.HoldingRegisterCount = 0;
    Protocol.HoldingFragmentCount = 0;
    
    // COMMANDS
    
    inverter.RegisterCommand("datetime/get", getDateTime);
    inverter.RegisterCommand("datetime/set", updateDateTime);
    
    inverter.RegisterCommand("batteryfirst/get", getBatteryFirst);
    inverter.RegisterCommand("batteryfirst/set/powerrate",
                             setBatteryFirstPowerRate);
    inverter.RegisterCommand("batteryfirst/set/stopsoc", setBatteryFirstStopSOC);
    inverter.RegisterCommand("batteryfirst/set/acchargeenabled",
                             setBatteryFirstACChargeEnabled);
    inverter.RegisterCommand("batteryfirst/set/timeslot",
                             setBatteryFirstTimeSlot);
    
    inverter.RegisterCommand("gridfirst/get", getGridFirst);
    inverter.RegisterCommand("gridfirst/set/powerrate", setGridFirstPowerRate);
    inverter.RegisterCommand("gridfirst/set/stopsoc", setGridFirstStopSOC);
    inverter.RegisterCommand("gridfirst/set/timeslot", setGridFirstTimeSlot);

    Log.print(F("init_growatt124: number of input registers "));
    Log.print(Protocol.InputRegisterCount);
    Log.print(F(" number of holding registers "));
    Log.println(Protocol.HoldingRegisterCount);
    #if ENABLE_VERSION124_IREG3000 == 1
    Log.println(F("init_growatt124: use of input registers 3000~3124,3125~3249"));
    #else
    Log.println(F("init_growatt124: use of input registers 0~124,1000~1124"));
    #endif
}
