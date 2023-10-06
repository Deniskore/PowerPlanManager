#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <fstream>
#include <array>
#include <memory>
#include <cstdint>
#include <optional>
#include "constants.h"
#include "utils.h"

constexpr auto CONFIG_NAME = L"\\powerplanmanager.cfg";

class Config : std::enable_shared_from_this<Config> {
public:
  Config() = default;
  ~Config() = default;
  Config(const Config &) = delete;
  Config(Config &&) = delete;

  Config &operator=(Config &&) = delete;
  Config &operator=(const Config &) = delete;

  static std::map<std::string, std::string> defaultConfig() {
    return {{"activationThreshold", std::to_string(ACTIVATION_THRESHOLD)},
            {"autostartup", std::to_string(false)},
            {"sleepInterval", std::to_string(SLEEP_INTERVAL_DEFAULT_MS)},
            {"nvidiaPerformanceMode", std::to_string(true)},
            {"playActivationSound", std::to_string(true)},
            {"digitalVibrance", std::to_string(DIGITAL_VIBRANCE)},
            {"nvidiaAnsel", std::to_string(false)},
            {"nvidiaVsync", std::to_string(false)},
            {"nvidiaLowLatencyMode", std::to_string(false)},
            {"nvidiaUltraLowLatencyMode", std::to_string(false)},
            {"alwaysNvidiaPerformanceMode", std::to_string(false)},
            {"alwaysCPUPerformanceMode", std::to_string(false)},
            {"alwaysVibranceLevel", std::to_string(false)}};
  }

  void WriteConfig();
  void ReloadConfig();
  void
  AssignConfigParameters(const std::map<std::string, std::string> &configMap);
  static std::shared_ptr<Config> ReadConfig();
  static std::optional<std::map<std::string, std::string>>
  ParseConfig(const std::string &input);

  static std::optional<std::shared_ptr<Config>>
  ParseConfigMap(const std::map<std::string, std::string> &configMap);

public:
  std::atomic<uint32_t> activationThreshold{ACTIVATION_THRESHOLD};
  std::atomic<int32_t> digitalVibranceLevel{DIGITAL_VIBRANCE};
  std::atomic<uint32_t> sleepInterval{SLEEP_INTERVAL_DEFAULT_MS};
  std::atomic<bool> autoStartup{false};
  std::atomic<bool> playActivationSound{true};
  std::atomic<bool> nvidiaAnsel{false};
  std::atomic<bool> nvidiaVsync{false};
  std::atomic<bool> nvidiaPerformanceMode{true};
  std::atomic<bool> nvidiaLowLatencyMode{false};
  std::atomic<bool> nvidiaUltraLowLatencyMode{false};
  std::atomic<bool> alwaysNvidiaPerformanceMode{false};
  std::atomic<bool> alwaysCPUPerformanceMode{false};
  std::atomic<bool> alwaysVibranceLevel{false};
};
