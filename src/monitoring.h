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

class Monitoring {
public:
  Monitoring() = delete;
  ~Monitoring() = delete;
  Monitoring(const Monitoring &) = delete;
  Monitoring(Monitoring &&) = delete;
  Monitoring &operator=(Monitoring &&) = delete;
  Monitoring &operator=(const Monitoring &) = delete;

  static bool CheckForeGroundWindow(HWND &fullscreenWindow,
                                    std::shared_ptr<Config> &config);
  static void MonitoringThread(const std::stop_token &stop_token,
                               std::shared_ptr<Config> config,
                               std::atomic<bool> &pauseCFGWatcher);
  static void CFGMonitoringThread(const std::stop_token &stop_token,
                                  std::shared_ptr<Config> config,
                                  std::atomic<bool> &pauseCFGWatcher);
};
