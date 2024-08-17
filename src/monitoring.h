#pragma once

#include <Windows.h>
#include <shellapi.h>
#include <atomic>
#include <thread>
#include "cpu.h"
#include "config.h"
#include "constants.h"
#include "power.h"
#include "utils.h"
#include "gpu.h"

namespace Monitoring {
  bool CheckForeGroundWindow(HWND &fullscreenWindow,
                             std::shared_ptr<Config> &config);
  void MonitoringThread(const std::stop_token &stop_token,
                        std::shared_ptr<Config> config,
                        std::atomic<bool> &pauseCFGWatcher);
  void CFGMonitoringThread(const std::stop_token &stop_token,
                           std::shared_ptr<Config> config,
                           std::atomic<bool> &pauseCFGWatcher);
}; // namespace Monitoring
