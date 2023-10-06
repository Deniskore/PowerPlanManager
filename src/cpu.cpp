#include "cpu.h"

double CPU::GetUtilization() {
  PDH_HQUERY cpuQuery = {};
  PDH_HCOUNTER cpuCounter = {};
  PDH_STATUS status = {};

  status = PdhOpenQuery(nullptr, 0, &cpuQuery);

  if (status != ERROR_SUCCESS) {
    return -1;
  }

  status = PdhAddCounter(
      cpuQuery, L"\\Processor Information(_Total)\\% Processor Utility", 0,
      &cpuCounter);

  if (status != ERROR_SUCCESS) {
    return -1;
  }

  // Collect a sample and sleep for 5 seconds
  PdhCollectQueryData(cpuQuery);
  Sleep(PDH_SLEEP_MS);
  PdhCollectQueryData(cpuQuery);

  PDH_FMT_COUNTERVALUE counterValue;
  status = PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, nullptr,
                                       &counterValue);
  if (status != ERROR_SUCCESS) {
    return -1;
  }

  PdhCloseQuery(cpuQuery);

  return counterValue.doubleValue;
}

// Check AMD 2xxx, 3xxx, 4xxx.
bool CPU::IsRyzen2k3k4k() {
  constexpr int32_t EXTENDED_INFORMATION = 0x80000000u;
  constexpr int32_t START_PROCESSOR_BRAND = 0x80000002u;
  constexpr int32_t END_PROCESSOR_BRAND = 0x80000004u;
  constexpr int DATA_SEGMENT_SIZE = 16;
  // 3 calls * 4 registers per call * 4 bytes per register = 48 bytes
  constexpr int CPU_BRAND_SIZE = 48;

  std::array<int, 4> data = {0};
  std::array<char, CPU_BRAND_SIZE> brand = {0};

  __cpuid(data.data(), EXTENDED_INFORMATION);
  if (data[0] >= END_PROCESSOR_BRAND) {
    for (std::int32_t i = START_PROCESSOR_BRAND; i <= END_PROCESSOR_BRAND;
         ++i) {
      __cpuid(data.data(), i);
      std::memcpy(brand.data() +
                      DATA_SEGMENT_SIZE * static_cast<std::ptrdiff_t>(
                                              i - START_PROCESSOR_BRAND),
                  data.data(), sizeof(data));
    }
  }

  // this can also be done using id of models for Ryzen which has a custom power
  // plan, but this is quite a large list of models
  const std::string brandStr(brand.data());
  const static std::regex rx(R"(AMD Ryzen [0-9]+ 3[0-9]{3}x?)");
  return std::regex_search(brandStr, rx);
}
