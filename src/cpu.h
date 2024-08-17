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

namespace CPU {
  double GetUtilization();
  bool IsRyzen2k3k4k();
}; // namespace CPU
