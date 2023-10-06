#pragma once

#include <Windows.h>
#include <powersetting.h>
#include <PowrProf.h>
#include <Guiddef.h>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <string>
#include "constants.h"
#include "cpu.h"

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

class PowerManager {
public:
  PowerManager() = delete;
  ~PowerManager() = delete;
  PowerManager(const PowerManager &) = delete;
  PowerManager(PowerManager &&) = delete;
  PowerManager &operator=(PowerManager &&) = delete;
  PowerManager &operator=(const PowerManager &) = delete;

  static void TogglePowerPlan(bool enableHighPerformance);
  static void SetPowerPlan(GUID guid);
  static GUID GetPowerPlan();
  static std::wstring GetPowerSchemeName(GUID *pPowerSchemeGuid);
  static std::vector<std::tuple<GUID, std::wstring>> EnumeratePowerSchemes();
  static GUID GetRyzenPerfGUID();
  static GUID GetRyzenBalancedGUID();
};
