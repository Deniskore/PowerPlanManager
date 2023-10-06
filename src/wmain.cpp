#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <string>
#include <thread>
#include <memory>
#include "resources.h"
#include "cpu.h"
#include "monitoring.h"
#include "constants.h"
#include "power.h"
#include "utils.h"
#include "config.h"
#include "gpu.h"
#include "gui.h"

bool CheckParameter(LPWSTR *argv, int argc, const std::wstring &parameter) {
  for (int i = 0; i < argc; ++i) {
    if (parameter == argv[i]) { // NOLINT: pointer arithmetic
      return true;
    }
  }

  return false;
}

// TODO: IMPLEMENT about dialog
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
  int32_t argc = 0;
  LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

  if (CheckParameter(argv, argc, L"/autostartup")) {
    Utils::AddToStartup(Utils::GetExecutablePath());
    ExitProcess(EXIT_SUCCESS);
  } else if (CheckParameter(argv, argc, L"/removeautostartup")) {
    Utils::RemoveFromStartup();
    ExitProcess(EXIT_SUCCESS);
  }

  LocalFree(argv);

  HANDLE mutexHandle = CreateMutexA(nullptr, TRUE, "PowerPlanManagerMutex");

  // Check if the named mutex already exists
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBox(nullptr, L"Program is already running!", L"Error",
               MB_ICONERROR | MB_OK);
    return EXIT_SUCCESS;
  }

  ApplySettings();

  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = L"PowerPlanWindowClass";

  if (!RegisterClassEx(&wc)) {
    MessageBox(nullptr, L"Error registering window class", L"Error",
               MB_ICONEXCLAMATION | MB_OK);
    return 1;
  }
  auto windowData = std::make_shared<SharedWindowData>();
  auto cfg = Config::ReadConfig();

  windowData->config = cfg.get();
  windowData->dialogType = DIALOG_TYPE_NONE;
  windowData->lastWindow = nullptr;

  HWND hWnd =
      CreateWindowEx(0, wc.lpszClassName, L"PowerPlanManagerTray", 0, 0, 0, 0,
                     0, HWND_MESSAGE, nullptr, hInstance, windowData.get());

  if (!hWnd) {
    MessageBox(nullptr, L"Error creating PowerPlanManagerTray window", L"Error",
               MB_ICONEXCLAMATION | MB_OK);
    return 1;
  }

  NOTIFYICONDATA nid = {};
  nid.cbSize = sizeof(NOTIFYICONDATA);
  nid.uVersion = NOTIFYICON_VERSION_4;
  nid.hWnd = hWnd;
  nid.uID = UID;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
  nid.uCallbackMessage = WM_APP + 1;
  nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
  StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), L"PowerPlan Manager");
  Shell_NotifyIcon(NIM_ADD, &nid);

  std::atomic<bool> pauseCFGWatcher(false);
  std::jthread monitoringThread(Monitoring::MonitoringThread, cfg, std::ref(pauseCFGWatcher));
  monitoringThread.detach();

  std::jthread cfgMonitoringThread(Monitoring::CFGMonitoringThread, cfg, std::ref(pauseCFGWatcher));
  cfgMonitoringThread.detach();

  // Process window messages.
  MSG msg = {};
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Remove tray icon.
  Shell_NotifyIcon(NIM_DELETE, &nid);

  monitoringThread.request_stop();
  cfgMonitoringThread.request_stop();

  if (!windowData->config->alwaysNvidiaPerformanceMode.load(
          std::memory_order_relaxed)) {
    GPU::ToggleNvidiaPerformanceMode(false);
  }

  if (!windowData->config->alwaysCPUPerformanceMode.load(
          std::memory_order_relaxed)) {
    PowerManager::TogglePowerPlan(false);
  }

  if (!windowData->config->alwaysVibranceLevel.load(
          std::memory_order_relaxed)) {
    GPU::NvidiaSetDigitalVibrance(0);
  }

  if (mutexHandle) {
    ReleaseMutex(mutexHandle);
    CloseHandle(mutexHandle);
  }

  return EXIT_SUCCESS;
}
