#pragma once

#include <windows.h>
#include <atomic>
#include <vector>
#include <memory>
#include "config.h"
#include "gpu.h"
#include "monitoring.h"
#include "resources.h"

enum DialogType {
  DIALOG_TYPE_NONE,
  DIALOG_TYPE_DIGITAL_VIBRANCE,
  DIALOG_TYPE_THRESHOLD,
};

auto constexpr THRESHOLD_CAPTION = L"CPU activation threshold";
auto constexpr DIGITAL_VIBRANCE_CAPTION = L"Digital vibrance level";

struct SharedWindowData : std::enable_shared_from_this<SharedWindowData> {
  Config *config;
  DialogType dialogType;
  HWND lastWindow;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ApplySettings();
