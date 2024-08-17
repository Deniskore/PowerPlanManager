#pragma once

#include <Windows.h>

#include "nvapi_interface.h"
#include "nvapi_inc_nvapi.h"
#include "NvApiDriverSettings.h"
#include "utils.h"

using NV_DISPLAY_DVC_INFO = struct {
  unsigned int version;
  int currentDV;
  int minDV;
  int maxDV;
};

using NvAPI_Initialize_t = NvAPI_Status(__cdecl *)();
using NvAPI_DRS_CreateSession_t = NvAPI_Status(__cdecl *)(NvDRSSessionHandle *);
using NvAPI_DRS_LoadSettings_t =
    NvAPI_Status(__cdecl *)(NvDRSSessionHandle session);
using NvAPI_DRS_GetCurrentGlobalProfile_t = NvAPI_Status(__cdecl *)(
    NvDRSSessionHandle session, NvDRSProfileHandle *profile);
using NvAPI_DRS_SetSetting_t =
    NvAPI_Status(__cdecl *)(NvDRSSessionHandle session,
                            NvDRSProfileHandle profile, NVDRS_SETTING *setting);
using NvAPI_DRS_SaveSettings_t =
    NvAPI_Status(__cdecl *)(NvDRSSessionHandle session);
using NvAPI_DRS_DestroySession_t =
    NvAPI_Status(__cdecl *)(NvDRSSessionHandle session);
using NvAPI_QueryInterface_t = void *(*)(uint32_t index);

using NvAPI_Unload_t = NvAPI_Status(__cdecl *)();

using NvAPI_EnumNvidiaDisplayHandle_t =
    NvAPI_Status(__cdecl *)(int thisEnum, NvDisplayHandle *handle);
using NvAPI_SetDVCLevel_t = NvAPI_Status(__cdecl *)(NvDisplayHandle handle,
                                                    int outputId, int level);
using NvAPI_GetDVCInfo_t = NvAPI_Status(__cdecl *)(
    NvDisplayHandle displayHandle, int outputId, NV_DISPLAY_DVC_INFO *DVCInfo);

static uint32_t findFunctionId(const char *functionName) {
  if (auto it = nvapi_interface_table.find(std::string(functionName));
      it != nvapi_interface_table.end()) {
    return it->second;
  }

  return 0;
}

// NOLINTBEGIN: cppcoreguidelines-special-member-functions
class NVDRSSession {
public:
  // If you constantly unload and load the nvidia library, there will be memory
  // leaks in the nvidia api, the only normal way to handle this is to keep the
  // nvidia session on throughout the program.
  static NVDRSSession &GetInstance() {
    static NVDRSSession instance;
    return instance;
  }

  NVDRSSession(NVDRSSession const &) = delete;

  ~NVDRSSession() {
    if (hSession) {
      NvAPI_DRS_DestroySession(hSession);
      NvAPI_Unload();
    }
  }

  void operator=(NVDRSSession const &) = delete;

  NvAPI_Status SetSetting(NVDRS_SETTING setting);
  NV_DISPLAY_DVC_INFO GetDVCInfo(NvDisplayHandle handle);
  NvAPI_Status SetDigitalVibrance(int desiredDVLevel);

  bool Initialized() { return this->initialized; }

private:
  NVDRSSession() {
#ifdef _WIN64
    hNvapiLib = std::move(ModuleHandle(L"nvapi64.dll"));
#else
    hNvapiLib = std::move(ModuleHandle(L"nvapi.dll"));
#endif

    NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress(
        hNvapiLib.get(), "nvapi_QueryInterface");

    NvAPI_Initialize = (NvAPI_Initialize_t)NvAPI_QueryInterface(
        findFunctionId("NvAPI_Initialize"));
    NvAPI_DRS_CreateSession = (NvAPI_DRS_CreateSession_t)NvAPI_QueryInterface(
        findFunctionId("NvAPI_DRS_CreateSession"));
    NvAPI_DRS_LoadSettings = (NvAPI_DRS_LoadSettings_t)NvAPI_QueryInterface(
        findFunctionId("NvAPI_DRS_LoadSettings"));
    NvAPI_DRS_GetCurrentGlobalProfile =
        (NvAPI_DRS_GetCurrentGlobalProfile_t)NvAPI_QueryInterface(
            findFunctionId("NvAPI_DRS_GetBaseProfile"));
    NvAPI_DRS_SetSetting = (NvAPI_DRS_SetSetting_t)NvAPI_QueryInterface(
        findFunctionId("NvAPI_DRS_SetSetting"));
    NvAPI_DRS_SaveSettings = (NvAPI_DRS_SaveSettings_t)NvAPI_QueryInterface(
        findFunctionId("NvAPI_DRS_SaveSettings"));
    NvAPI_DRS_DestroySession = (NvAPI_DRS_DestroySession_t)NvAPI_QueryInterface(
        findFunctionId("NvAPI_DRS_DestroySession"));
    NvAPI_Unload =
        (NvAPI_Unload_t)NvAPI_QueryInterface(findFunctionId("NvAPI_Unload"));
    NvAPI_EnumNvidiaDisplayHandle =
        (NvAPI_EnumNvidiaDisplayHandle_t)NvAPI_QueryInterface(
            findFunctionId("NvAPI_EnumNvidiaDisplayHandle"));
    NvAPI_SetDVCLevel = (NvAPI_SetDVCLevel_t)NvAPI_QueryInterface(
        findFunctionId("NvAPI_SetDVCLevel"));
    NvAPI_GetDVCInfo = (NvAPI_GetDVCInfo_t)NvAPI_QueryInterface(
        findFunctionId("NvAPI_GetDVCInfo"));

    if (!NvAPI_Initialize || !NvAPI_DRS_CreateSession ||
        !NvAPI_DRS_LoadSettings || !NvAPI_DRS_GetCurrentGlobalProfile ||
        !NvAPI_DRS_SetSetting || !NvAPI_DRS_SaveSettings ||
        !NvAPI_DRS_DestroySession || !NvAPI_Unload ||
        !NvAPI_EnumNvidiaDisplayHandle || !NvAPI_SetDVCLevel ||
        !NvAPI_GetDVCInfo) {
      return;
    }

    NvAPI_Status status = NvAPI_Initialize();
    if (status != NVAPI_OK) {
      return;
    }

    status = NvAPI_DRS_CreateSession(&hSession);
    if (status != NVAPI_OK) {
      return;
    }

    status = NvAPI_DRS_LoadSettings(hSession);
    if (status != NVAPI_OK) {
      return;
    }

    status = NvAPI_DRS_GetCurrentGlobalProfile(hSession, &hGlobalProfile);

    if (status != NVAPI_OK) {
      return;
    }

    initialized = true;
  }

  bool initialized = false;
  ModuleHandle hNvapiLib;
  NvDRSProfileHandle hGlobalProfile;
  NvDRSSessionHandle hSession;
  NvAPI_QueryInterface_t NvAPI_QueryInterface;
  NvAPI_Initialize_t NvAPI_Initialize;
  NvAPI_DRS_CreateSession_t NvAPI_DRS_CreateSession;
  NvAPI_DRS_LoadSettings_t NvAPI_DRS_LoadSettings;
  NvAPI_DRS_GetCurrentGlobalProfile_t NvAPI_DRS_GetCurrentGlobalProfile;
  NvAPI_DRS_SetSetting_t NvAPI_DRS_SetSetting;
  NvAPI_DRS_SaveSettings_t NvAPI_DRS_SaveSettings;
  NvAPI_DRS_DestroySession_t NvAPI_DRS_DestroySession;
  NvAPI_Unload_t NvAPI_Unload;
  NvAPI_EnumNvidiaDisplayHandle_t NvAPI_EnumNvidiaDisplayHandle;
  NvAPI_SetDVCLevel_t NvAPI_SetDVCLevel;
  NvAPI_GetDVCInfo_t NvAPI_GetDVCInfo;
};

// NOLINTEND: cppcoreguidelines-special-member-functions

enum GPULatencyType {
  Off = 0,
  On = 1,
  Ultra = 2,
};

namespace GPU {

  NvAPI_Status ToggleNvidiaPerformanceMode(bool enablePerformanceMode);
  NvAPI_Status ToggleNvidiaAnsel(bool enableAnsel);
  NvAPI_Status ToggleNvidiaVsync(bool enableVsync);
  NvAPI_Status ToggleNvidiaLowLatencyMode(bool enableLowLatencyMode);
  NvAPI_Status ToggleNvidiaUltraLowLatencyMode(bool enableUltraLowLatencyMode);
  NvAPI_Status NvidiaLowLatencyModeCPL(GPULatencyType latencyState);
  NvAPI_Status NvidiaSetDigitalVibrance(int desiredDVLevel);

  NvAPI_Status NvidiaMaxPrerenderedFrames(int maxPrerenderedFrames);
}; // namespace GPU
