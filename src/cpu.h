#pragma once

#include <pdh.h>
#include <pdhmsg.h>
#include <cstdint>
#include <array>
#include <string>
#include <intrin.h>
#include <algorithm>
#include <regex>
#include "constants.h"

class CPU {
public:
  CPU() = delete;
  ~CPU() = delete;
  CPU(const CPU &) = delete;
  CPU(CPU &&) = delete;
  CPU &operator=(CPU &&) = delete;
  CPU &operator=(const CPU &) = delete;

  static double GetUtilization();
  static bool IsRyzen2k3k4k();
};
