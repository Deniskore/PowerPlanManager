#pragma once

#include <cstdint>
#include <guiddef.h>

constexpr uint32_t ACTIVATION_THRESHOLD_SETTING = 1002;
constexpr uint32_t DIGITAL_VIBRANCE_SETTING = 1003;
constexpr uint32_t EXIT_APP = 1004;
constexpr uint32_t AUTOSTART = 1005;
constexpr uint32_t NVIDIA_PERFORMANCE_MODE = 1006;
constexpr uint32_t NVIDIA_ANSEL = 1007;
constexpr uint32_t NVIDIA_VSYNC = 1008;
constexpr uint32_t NVIDIA_LOW_LATENCY_MODE = 1009;
constexpr uint32_t NVIDIA_ULTRA_LOW_LATENCY_MODE = 1010;
constexpr uint32_t ALWAYS_MINIMUM_TIMER_RESOLUTION = 1011;
constexpr uint32_t ALWAYS_CPU_PERFORMANCE_MODE = 1012;
constexpr uint32_t ALWAYS_NVIDIA_PERFORMANCE_MODE = 1013;
constexpr uint32_t ALWAYS_VIBRANCE_LEVEL = 1014;
constexpr uint32_t PLAY_ACTIVATION_SOUND = 1015;

constexpr int32_t SLEEP_INTERVAL_DEFAULT_MS = 10000;
constexpr int32_t ACTIVATION_THRESHOLD = 10;
constexpr int32_t DIGITAL_VIBRANCE = 0;
constexpr int32_t UID = 100;
constexpr int32_t PARAMETER = 1000;

constexpr int32_t PDH_SLEEP_MS = 5000;

// clang-format off
constexpr GUID GUID_HIGH_PERFORMANCE = { 0x8c5e7fda, 0xe8bf, 0x4a96, { 0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8c, 0x63, 0x5c } };
constexpr GUID GUID_BALANCED = { 0x381b4222, 0xf694, 0x41f0, { 0x96, 0x85, 0xff, 0x5b, 0xb2, 0x60, 0xdf, 0x2e } };
constexpr GUID GUID_POWER_SAVER = { 0xa1841308, 0x3541, 0x4fab, { 0xbc, 0x81, 0xf7, 0x15, 0x56, 0xf2, 0x0b, 0x4a } };
// clang-format on
