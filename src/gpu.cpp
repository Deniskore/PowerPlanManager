#include "gpu.h"

namespace GPU {

  NvAPI_Status ToggleNvidiaPerformanceMode(bool enablePerformanceMode) {

    auto &session = NVDRSSession::GetInstance();

    if (!session.Initialized()) {
      return NVAPI_ERROR;
    }

    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = PREFERRED_PSTATE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.settingLocation = NVDRS_GLOBAL_PROFILE_LOCATION;
    setting.isCurrentPredefined = 0;
    setting.isPredefinedValid = 0;

    if (enablePerformanceMode) {
      setting.u32CurrentValue = PREFERRED_PSTATE_PREFER_MAX;
      setting.u32PredefinedValue = PREFERRED_PSTATE_PREFER_MAX;
    } else {
      setting.u32CurrentValue = PREFERRED_PSTATE_DEFAULT;
      setting.u32PredefinedValue = PREFERRED_PSTATE_DEFAULT;
    }

    return session.SetSetting(setting);
  }

  NvAPI_Status ToggleNvidiaAnsel(bool enableAnsel) {
    auto &session = NVDRSSession::GetInstance();

    if (!session.Initialized()) {
      return NVAPI_ERROR;
    }

    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = ANSEL_ENABLE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.settingLocation = NVDRS_GLOBAL_PROFILE_LOCATION;
    setting.isCurrentPredefined = 0;
    setting.isPredefinedValid = 0;

    if (enableAnsel) {
      setting.u32CurrentValue = ANSEL_ENABLE_DEFAULT;
      setting.u32PredefinedValue = ANSEL_ENABLE_DEFAULT;
    } else {
      setting.u32CurrentValue = ANSEL_ENABLE_OFF;
      setting.u32PredefinedValue = ANSEL_ENABLE_OFF;
    }

    return session.SetSetting(setting);
  }

  NvAPI_Status ToggleNvidiaVsync(bool enableVsync) {

    auto &session = NVDRSSession::GetInstance();

    if (!session.Initialized()) {
      return NVAPI_ERROR;
    }

    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = VSYNCMODE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.settingLocation = NVDRS_GLOBAL_PROFILE_LOCATION;
    setting.isCurrentPredefined = 0;
    setting.isPredefinedValid = 0;

    if (enableVsync) {
      setting.u32CurrentValue = VSYNCMODE_FORCEON;
      setting.u32PredefinedValue = VSYNCMODE_FORCEON;
    } else {
      setting.u32CurrentValue = VSYNCMODE_FORCEOFF;
      setting.u32PredefinedValue = VSYNCMODE_FORCEOFF;
    }

    return session.SetSetting(setting);
  }

  // Just change the state of the CPL dropdown menu, not the actual low latency
  // mode
  NvAPI_Status NvidiaLowLatencyModeCPL(GPULatencyType latencyState) {
    auto &session = NVDRSSession::GetInstance();

    if (!session.Initialized()) {
      return NVAPI_ERROR;
    }

    constexpr auto CPL_LOW_LATENCY_ID = 0x0005F543;

    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = CPL_LOW_LATENCY_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.settingLocation = NVDRS_GLOBAL_PROFILE_LOCATION;
    setting.isCurrentPredefined = NV_FALSE;
    setting.isPredefinedValid = NV_FALSE;

    switch (latencyState) {
    case GPULatencyType::Off:
      setting.u32CurrentValue = 0;
      break;
    case GPULatencyType::On:
      setting.u32CurrentValue = 1;
      break;
    case GPULatencyType::Ultra:
      setting.u32CurrentValue = 2;
      break;
    }

    return session.SetSetting(setting);
  }

  NvAPI_Status NvidiaSetDigitalVibrance(int desiredDVLevel) {
    auto &session = NVDRSSession::GetInstance();

    if (!session.Initialized()) {
      return NVAPI_ERROR;
    }

    return session.SetDigitalVibrance(desiredDVLevel);
  }

  NvAPI_Status NvidiaMaxPrerenderedFrames(int maxPrerenderedFrames) {
    auto &session = NVDRSSession::GetInstance();

    if (!session.Initialized()) {
      return NVAPI_ERROR;
    }

    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = PRERENDERLIMIT_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.settingLocation = NVDRS_GLOBAL_PROFILE_LOCATION;
    setting.isCurrentPredefined = NV_FALSE;
    setting.isPredefinedValid = NV_FALSE;

    setting.u32CurrentValue = maxPrerenderedFrames;

    return session.SetSetting(setting);
  }

  NvAPI_Status ToggleNvidiaLowLatencyMode(bool enableLowLatencyMode) {
    const GPULatencyType latency =
        enableLowLatencyMode ? GPULatencyType::On : GPULatencyType::Off;
    const int frameCount = enableLowLatencyMode ? 1 : 0;

    if (NvidiaLowLatencyModeCPL(latency) != NVAPI_OK ||
        NvidiaMaxPrerenderedFrames(frameCount) != NVAPI_OK) {
      return NVAPI_ERROR;
    }

    return NVAPI_OK;
  }

  NvAPI_Status ToggleNvidiaUltraLowLatencyMode(bool enableLowLatencyMode) {

    const GPULatencyType type =
        enableLowLatencyMode ? GPULatencyType::Ultra : GPULatencyType::Off;
    const int maxPrerenderedFrames = enableLowLatencyMode ? 1 : 0;

    if (NvidiaLowLatencyModeCPL(type) != NVAPI_OK ||
        NvidiaMaxPrerenderedFrames(maxPrerenderedFrames) != NVAPI_OK) {
      return NVAPI_ERROR;
    }

    auto &session = NVDRSSession::GetInstance();

    if (!session.Initialized()) {
      return NVAPI_ERROR;
    }

    constexpr auto ULTRA_LOW_LATENCY_MODE_ID = 0x10835000;

    NVDRS_SETTING setting = {};
    setting.version = NVDRS_SETTING_VER;
    setting.settingId = ULTRA_LOW_LATENCY_MODE_ID;
    setting.settingType = NVDRS_DWORD_TYPE;
    setting.settingLocation = NVDRS_GLOBAL_PROFILE_LOCATION;
    setting.isCurrentPredefined = NV_FALSE;
    setting.isPredefinedValid = NV_FALSE;

    if (enableLowLatencyMode) {
      setting.u32CurrentValue = 1;
    } else {
      setting.u32CurrentValue = 0;
    }

    return session.SetSetting(setting);
  }
} // namespace GPU

NvAPI_Status NVDRSSession::SetSetting(NVDRS_SETTING setting) {
  if (NvAPI_DRS_SetSetting(hSession, hGlobalProfile, &setting) != NVAPI_OK) {
    return NVAPI_ERROR;
  }
  if (NvAPI_DRS_SaveSettings(hSession) != NVAPI_OK) {
    return NVAPI_ERROR;
  }
  return NVAPI_OK;
}

NV_DISPLAY_DVC_INFO NVDRSSession::GetDVCInfo(NvDisplayHandle handle) {

  NV_DISPLAY_DVC_INFO info = {};
  info.version = sizeof(NV_DISPLAY_DVC_INFO) | 0x10000; // NOLINT

  NvAPI_GetDVCInfo(handle, 0, &info);

  return info;
}

NvAPI_Status NVDRSSession::SetDigitalVibrance(int desiredDVLevel) {
  NvDisplayHandle handle = nullptr;

  const NvAPI_Status status = NvAPI_EnumNvidiaDisplayHandle(0, &handle);

  if (status != NVAPI_OK) {
    return status;
  }

  const NV_DISPLAY_DVC_INFO info = GetDVCInfo(handle);
  int desiredDV = 0;

  if (desiredDVLevel > info.maxDV) {
    desiredDV = info.maxDV;
  } else if (desiredDVLevel < info.minDV) {
    desiredDV = info.minDV;
  } else {
    desiredDV = desiredDVLevel;
  }

  // Map the slider value (from the range [0, 100]) to the range [minDV,
  // maxDV]
  const int DV = info.minDV + (int)((double)desiredDVLevel *
                                    (double)(info.maxDV - info.minDV) / 100.0);

  return NvAPI_SetDVCLevel(handle, 0, DV);
}
