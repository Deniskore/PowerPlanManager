#include "config.h"

void Config::AssignConfigParameters(
    const std::map<std::string, std::string> &configMap) {
  // Lambda functions for conversions
  auto toBool = [](const std::string &str) { return std::stoi(str) != 0; };
  auto toInt = [](const std::string &str) { return std::stoi(str); };
  auto toULong = [](const std::string &str) { return std::stoul(str); };

  try {
    this->activationThreshold = toULong(configMap.at("activation_threshold"));
    this->digitalVibranceLevel = toInt(configMap.at("digital_vibrance"));
    this->autoStartup = toBool(configMap.at("autostartup"));
    this->sleepInterval = toULong(configMap.at("sleep_interval_ms"));
    this->nvidiaPerformanceMode =
        toBool(configMap.at("nvidia_performance_mode"));
    this->playActivationSound = toBool(configMap.at("play_activation_sound"));
    this->nvidiaAnsel = toBool(configMap.at("nvidia_ansel"));
    this->nvidiaVsync = toBool(configMap.at("nvidia_vsync"));
    this->nvidiaLowLatencyMode =
        toBool(configMap.at("nvidia_low_latency_mode"));
    this->nvidiaUltraLowLatencyMode =
        toBool(configMap.at("nvidia_ultra_low_latency_mode"));
    this->alwaysCPUPerformanceMode =
        toBool(configMap.at("always_cpu_performance_mode"));
    this->alwaysNvidiaPerformanceMode =
        toBool(configMap.at("always_nvidia_performance_mode"));
    this->alwaysVibranceLevel = toBool(configMap.at("always_vibrance_level"));
  } catch (const std::exception &e) {
    MessageBox(nullptr, L"Error while parsing config file", L"Error",
               MB_ICONERROR);
  }
}

void Config::WriteConfig() {
  auto configFilePath = Utils::GetAppDataPath() + CONFIG_NAME;

  const std::map<std::string, std::string> configParams = {
      {"activation_threshold", std::to_string(this->activationThreshold)},
      {"digital_vibrance", std::to_string(this->digitalVibranceLevel)},
      {"autostartup", std::to_string(this->autoStartup)},
      {"sleep_interval_ms", std::to_string(this->sleepInterval)},
      {"nvidia_performance_mode", std::to_string(this->nvidiaPerformanceMode)},
      {"play_activation_sound", std::to_string(this->playActivationSound)},
      {"nvidia_ansel", std::to_string(this->nvidiaAnsel)},
      {"nvidia_vsync", std::to_string(this->nvidiaVsync)},
      {"nvidia_low_latency_mode", std::to_string(this->nvidiaLowLatencyMode)},
      {"nvidia_ultra_low_latency_mode",
       std::to_string(this->nvidiaUltraLowLatencyMode)},
      {"always_cpu_performance_mode",
       std::to_string(this->alwaysCPUPerformanceMode)},
      {"always_nvidia_performance_mode",
       std::to_string(this->alwaysNvidiaPerformanceMode)},
      {"always_vibrance_level", std::to_string(this->alwaysVibranceLevel)}};

  std::fstream configFile(configFilePath,
                          std::fstream::out | std::fstream::trunc);
  if (configFile) {
    for (auto &entry : configParams) {
      configFile << entry.first << " = \"" << entry.second << "\"\n";
    }
    configFile.close();
  }
}

void Config::ReloadConfig() {
  auto appDataPath = Utils::GetAppDataPath();
  auto configFilePath = appDataPath + CONFIG_NAME;

  const std::fstream file(configFilePath, std::ios::in);

  std::ostringstream fileContent;
  fileContent << file.rdbuf();

  auto parsedConfig = ParseConfig(fileContent.str());
  if (parsedConfig.has_value()) {
    AssignConfigParameters(parsedConfig.value());
  }
}

std::shared_ptr<Config> Config::ReadConfig() {
  auto appDataPath = Utils::GetAppDataPath();
  auto configFilePath = appDataPath + CONFIG_NAME;

  const std::fstream file(configFilePath, std::ios::in);

  if (!file.is_open()) {
    std::shared_ptr<Config> config = std::make_shared<Config>();
    config->WriteConfig();
    return config;
  }

  std::ostringstream fileContent;
  fileContent << file.rdbuf();

  auto parsedConfig =
      ParseConfig(fileContent.str()).value_or(Config::defaultConfig());
  return ParseConfigMap(parsedConfig).value_or(std::make_shared<Config>());
}

std::string StripSymbols(const std::string &str, const std::string &symbols) {
  std::string stripped = str;
  for (const char symbol : symbols) {
    stripped.erase(std::remove(stripped.begin(), stripped.end(), symbol),
                   stripped.end());
  }
  return stripped;
}

std::optional<std::map<std::string, std::string>>
Config::ParseConfig(const std::string &input) {
  std::istringstream iss(input);
  std::map<std::string, std::string> config_map;
  std::string line;

  while (std::getline(iss, line)) {
    const size_t start_pos = line.find_first_not_of(" \t");
    const size_t end_pos = line.find_first_of('=');

    if (start_pos != std::string::npos && end_pos != std::string::npos) {
      const std::string key_raw = line.substr(start_pos, end_pos - start_pos);
      const std::string value_raw =
          line.substr(line.find_first_not_of(" \t", end_pos + 1));

      const std::string key = StripSymbols(key_raw, " ");
      const std::string value = StripSymbols(value_raw, "\"\'");

      config_map[key] = value;
    } else {
      return std::nullopt;
    }
  }
  return config_map;
}

std::optional<std::shared_ptr<Config>>
Config::ParseConfigMap(const std::map<std::string, std::string> &config_map) {
  auto config = std::make_shared<Config>();
  try {
    config->AssignConfigParameters(config_map);
    return config;
  } catch (const std::exception &e) {
    return std::nullopt;
  }
}
