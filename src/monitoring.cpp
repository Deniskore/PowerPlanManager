#include "monitoring.h"

bool Monitoring::CheckForeGroundWindow(HWND &fullscreenWindow,
                                       std::shared_ptr<Config> &config) {
  HWND targetWindow = GetForegroundWindow();

  // If foreground window is not the same as last full screen window, then we
  // have to run additional checks
  if (targetWindow && (targetWindow != fullscreenWindow)) {
    if ((Utils::IsFullScreen(targetWindow) || Utils::IsD3D()) &&
        (CPU::GetUtilization() >
         config->activationThreshold.load(std::memory_order_relaxed))) {
      fullscreenWindow = targetWindow;
      return true;
    }
  }

  return false;
}

void Monitoring::MonitoringThread(const std::stop_token &stop_token,
                                  std::shared_ptr<Config> config,
                                  std::atomic<bool> &pauseCFGWatcher) {
  HWND fullscreenWindow = nullptr;

  while (!stop_token.stop_requested()) {
    // If full screen window is still alive, then we don't have to check
    // further
    if (!IsWindow(fullscreenWindow) &&
        CheckForeGroundWindow(fullscreenWindow, config)) {
      if (config->playActivationSound.load(std::memory_order_relaxed)) {
        Utils::PlayBubbleSound();
      }

      GPU::NvidiaSetDigitalVibrance(
          config->digitalVibranceLevel.load(std::memory_order_relaxed));

      GPU::ToggleNvidiaPerformanceMode(true);
      PowerManager::TogglePowerPlan(true);

      pauseCFGWatcher.store(true, std::memory_order_relaxed);
    } else if (!IsWindow(fullscreenWindow) && fullscreenWindow != nullptr) {
      if (config->playActivationSound.load(std::memory_order_relaxed)) {
        Utils::PlayBubbleSound();
      }

      if (!config->alwaysNvidiaPerformanceMode.load(
              std::memory_order_relaxed)) {
        GPU::ToggleNvidiaPerformanceMode(false);
      }

      if (!config->alwaysCPUPerformanceMode.load(std::memory_order_relaxed)) {
        PowerManager::TogglePowerPlan(false);
      }

      GPU::NvidiaSetDigitalVibrance(0);

      fullscreenWindow = nullptr;

      pauseCFGWatcher.store(false, std::memory_order_relaxed);
    }

    Sleep(config->sleepInterval.load(std::memory_order_relaxed));
  }
}

void Monitoring::CFGMonitoringThread(const std::stop_token &stop_token,
                                     std::shared_ptr<Config> config,
                                     std::atomic<bool> &pauseCFGWatcher) {
  const std::wstring path = Utils::GetAppDataPath();
  DWORD bytes = 0;
  constexpr uint32_t buffSize = 1024;
  std::array<char, buffSize> buffer = {};
  OVERLAPPED overlapped = {};

  HANDLE dir =
      CreateFileW(path.c_str(), FILE_LIST_DIRECTORY,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                  nullptr, OPEN_EXISTING,
                  FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

  if (dir == INVALID_HANDLE_VALUE) {
    return;
  }

  while (true) {
    if (pauseCFGWatcher.load(std::memory_order_relaxed) ||
        !ReadDirectoryChangesW(dir, buffer.data(), buffSize, true,
                               FILE_NOTIFY_CHANGE_LAST_WRITE, &bytes,
                               &overlapped, nullptr)) {
      Sleep(config->sleepInterval.load(std::memory_order_relaxed));
      continue;
    }

    Sleep(config->sleepInterval.load(std::memory_order_relaxed));

    auto notifyInfo = (FILE_NOTIFY_INFORMATION *)buffer.data();
    while (!stop_token.stop_requested()) {
      const std::wstring filename = std::wstring(
          notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));

      if (filename == L"powerplanmanager.cfg" &&
          notifyInfo->Action == FILE_ACTION_MODIFIED) {
        config->ReloadConfig();
      }

      if (notifyInfo->NextEntryOffset == 0)
        break;

      notifyInfo = (FILE_NOTIFY_INFORMATION *)((char *)(notifyInfo) + // NOLINT
                                               notifyInfo->NextEntryOffset);
    }
  }
}
