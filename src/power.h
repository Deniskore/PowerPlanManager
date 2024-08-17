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

namespace PowerManager {
  void TogglePowerPlan(bool enableHighPerformance);
  void SetPowerPlan(GUID guid);
  GUID GetPowerPlan();
  std::wstring GetPowerSchemeName(GUID *pPowerSchemeGuid);
  std::vector<std::tuple<GUID, std::wstring>> EnumeratePowerSchemes();
  GUID GetRyzenPerfGUID();
  GUID GetRyzenBalancedGUID();
}; // namespace PowerManager
