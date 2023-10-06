#include "power.h"

void PowerManager::TogglePowerPlan(bool enableHighPerformance) {
  const bool isAmdRyzen2k3k4k = CPU::IsRyzen2k3k4k();

  GUID powerPlan;
  if (isAmdRyzen2k3k4k) {
    powerPlan = enableHighPerformance ? GetRyzenPerfGUID()
                                      : GetRyzenBalancedGUID();
  } else {
    powerPlan = enableHighPerformance ? GUID_HIGH_PERFORMANCE : GUID_BALANCED;
  }

  PowerManager::SetPowerPlan(powerPlan);
}

void PowerManager::SetPowerPlan(GUID guid) {
  PowerSetActiveScheme(nullptr, &guid);
}

GUID PowerManager::GetPowerPlan() {
  GUID *guid = nullptr;

  if (PowerGetActiveScheme(nullptr, &guid) != ERROR_SUCCESS) {
    return {};
  }

  return *guid;
}

std::wstring PowerManager::GetPowerSchemeName(GUID *pPowerSchemeGuid) {
  DWORD bufferSize = 0;

  PowerReadFriendlyName(nullptr, pPowerSchemeGuid, nullptr, nullptr, nullptr,
                        &bufferSize);

  std::wstring friendlyName(bufferSize / sizeof(WCHAR), L'\0');
  if (ERROR_SUCCESS ==
      PowerReadFriendlyName(nullptr, pPowerSchemeGuid, nullptr, nullptr,
                            (PUCHAR)(friendlyName.data()), &bufferSize)) {
    return friendlyName;
  } else {
    return L"";
  }
}

std::vector<std::tuple<GUID, std::wstring>>
PowerManager::EnumeratePowerSchemes() {
  std::vector<std::tuple<GUID, std::wstring>> powerSchemes;

  GUID powerSchemeGuid;
  DWORD bufferSize = sizeof(GUID);
  int index = 0;

  while (ERROR_SUCCESS ==
         PowerEnumerate(nullptr, nullptr, nullptr, ACCESS_SCHEME, index,
                        (UCHAR *)(&powerSchemeGuid), &bufferSize)) {
    auto friendlyName = GetPowerSchemeName(&powerSchemeGuid);
    std::transform(friendlyName.begin(), friendlyName.end(),
                   friendlyName.begin(), ::tolower);
    powerSchemes.emplace_back(powerSchemeGuid, friendlyName);
    ++index;
  }

  return powerSchemes;
}

GUID PowerManager::GetRyzenPerfGUID() {
  auto powerSchemes = PowerManager::EnumeratePowerSchemes();
  for (const auto &[guid, name] : powerSchemes) {
    if (name.find(L"ryzen") != std::wstring::npos &&
        name.find(L"high") != std::wstring::npos) {
      return guid;
    }
  }
  // fall back to default high performance power plan
  return GUID_HIGH_PERFORMANCE;
}

GUID PowerManager::GetRyzenBalancedGUID() {
  auto powerSchemes = PowerManager::EnumeratePowerSchemes();
  for (const auto &[guid, name] : powerSchemes) {
    if (name.find(L"ryzen") != std::wstring::npos &&
        name.find(L"balanced") != std::wstring::npos) {
      return guid;
    }
  }
  // fall back to default balanced power plan
  return GUID_BALANCED;
}
