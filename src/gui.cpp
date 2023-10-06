
#include "gui.h"

void ApplySettings() {
  auto cfg = Config::ReadConfig();

  if (cfg->alwaysVibranceLevel) {
    GPU::NvidiaSetDigitalVibrance(cfg->digitalVibranceLevel.load(std::memory_order_relaxed));
  }

  GPU::ToggleNvidiaPerformanceMode(cfg->nvidiaPerformanceMode.load(std::memory_order_relaxed));
  GPU::ToggleNvidiaAnsel(cfg->nvidiaAnsel.load(std::memory_order_relaxed));
  GPU::ToggleNvidiaVsync(cfg->nvidiaVsync.load(std::memory_order_relaxed));
  GPU::ToggleNvidiaLowLatencyMode(cfg->nvidiaLowLatencyMode.load(std::memory_order_relaxed));
  GPU::ToggleNvidiaUltraLowLatencyMode(cfg->nvidiaUltraLowLatencyMode.load(std::memory_order_relaxed));
}

void ShowContextMenu(HWND hWnd, SharedWindowData *data) {
  POINT pt;
  GetCursorPos(&pt);
  HMENU hMenu = CreatePopupMenu();

  struct MenuElement {
    std::atomic_bool *configField;
    UINT uIDNewItem;
    LPCTSTR lpNewItem;
    bool addSeparator;
  };

  if (hMenu) {
    auto fieldStatus = [](std::atomic_bool *field) -> UINT {
      return field->load(std::memory_order_relaxed) ? MF_CHECKED : MF_UNCHECKED;
    };

    const std::vector<MenuElement> menuElements = {
        {&data->config->autoStartup, AUTOSTART, TEXT("Enable autostart"), true},
        {&data->config->nvidiaPerformanceMode, NVIDIA_PERFORMANCE_MODE,
         TEXT("Enable NVIDIA performance mode (Global Profile)"), false},
        {&data->config->nvidiaAnsel, NVIDIA_ANSEL,
         TEXT("Enable NVIDIA Ansel (Global Profile)"), false},
        {&data->config->nvidiaVsync, NVIDIA_VSYNC,
         TEXT("Enable NVIDIA V-Sync (Global Profile)"), false},
        {&data->config->nvidiaLowLatencyMode, NVIDIA_LOW_LATENCY_MODE,
         TEXT("Enable NVIDIA Low Latency Mode (Global Profile)"), false},
        {&data->config->nvidiaUltraLowLatencyMode,
         NVIDIA_ULTRA_LOW_LATENCY_MODE,
         TEXT("Enable NVIDIA Ultra Low Latency Mode (Global Profile)"), true},
        {&data->config->alwaysCPUPerformanceMode, ALWAYS_CPU_PERFORMANCE_MODE,
         TEXT("Always set system high performance plan"), false},
        {&data->config->alwaysNvidiaPerformanceMode,
         ALWAYS_NVIDIA_PERFORMANCE_MODE,
         TEXT("Always set NVIDIA high performance plan"), false},
        {&data->config->alwaysVibranceLevel, ALWAYS_VIBRANCE_LEVEL,
         TEXT("Always set digital vibrance level"), true},
        {&data->config->playActivationSound, PLAY_ACTIVATION_SOUND,
         TEXT("Play activation sound"), true},
    };

    for (const auto &element : menuElements) {
      AppendMenu(hMenu, MF_STRING | fieldStatus(element.configField),
                 element.uIDNewItem, element.lpNewItem);
      if (element.addSeparator) {
        AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
      }
    }

    AppendMenu(hMenu, MF_STRING, ACTIVATION_THRESHOLD_SETTING,
               TEXT("Configure activation threshold"));
    AppendMenu(hMenu, MF_STRING, DIGITAL_VIBRANCE_SETTING,
               TEXT("Configure digital vibrance"));
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, EXIT_APP, TEXT("Exit"));

    SetForegroundWindow(hWnd);
    const UINT uFlags =
        TPM_RIGHTBUTTON |
        (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0 ? TPM_RIGHTALIGN
                                                     : TPM_LEFTALIGN);
    TrackPopupMenuEx(hMenu, uFlags, pt.x, pt.y, hWnd, nullptr);
    DestroyMenu(hMenu);
  }
}

void UpdateSliderValueText(HWND hDlg, uint32_t value,
                           const std::wstring &text) {
  const std::wstring textValue = text + std::to_wstring(value);
  RECT rcDialog = {};
  GetClientRect(hDlg, &rcDialog);

  SIZE sizeText = {};
  HDC hdcStatic = GetDC(GetDlgItem(hDlg, IDC_SLIDER_VALUE));
  GetTextExtentPoint32(hdcStatic, textValue.c_str(),
                       static_cast<int>(textValue.length()), &sizeText);
  ReleaseDC(GetDlgItem(hDlg, IDC_SLIDER_VALUE), hdcStatic);

  const int xPos = (rcDialog.right - sizeText.cx) / 2;
  const int yPos = 5;
  SetWindowPos(GetDlgItem(hDlg, IDC_SLIDER_VALUE), nullptr, xPos, yPos, 0, 0,
               SWP_NOSIZE | SWP_NOZORDER);
  SetDlgItemText(hDlg, IDC_SLIDER_VALUE, textValue.c_str());
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam,
                            LPARAM lParam) {
  SharedWindowData *data = nullptr;
  constexpr auto endRange = 100;
  std::wstring dialogCaption, sliderMessage;
  auto getValue = [&](auto &dataConfigVal) -> int32_t {
    return dataConfigVal.load(std::memory_order_relaxed);
  };
  auto setValue = [&](auto &dataConfigVal, int32_t currentPos) {
    dataConfigVal.store(currentPos, std::memory_order_relaxed);
  };

  switch (message) {
  case WM_INITDIALOG: {
    int32_t initRange = 0, initValue = 0;

    data = ((SharedWindowData *)lParam);
    if (data->dialogType == DIALOG_TYPE_THRESHOLD) {
      dialogCaption = THRESHOLD_CAPTION;
      sliderMessage = L"Activation threshold: ";
      initRange = ACTIVATION_THRESHOLD;
      initValue = getValue(data->config->activationThreshold);
    } else {
      dialogCaption = DIGITAL_VIBRANCE_CAPTION;
      sliderMessage = L"Select digital vibrance level: ";
      initValue = getValue(data->config->digitalVibranceLevel);
    }

    data->lastWindow = hDlg;
    SetWindowText(hDlg, dialogCaption.data());
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)data);
    SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_SETRANGE, TRUE,
                       MAKELONG(initRange, endRange));
    SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_SETPOS, TRUE, initValue);
    UpdateSliderValueText(hDlg, initValue, sliderMessage.data());

    return TRUE;
  }

  case WM_HSCROLL: {
    data = ((SharedWindowData *)(GetWindowLongPtr(hDlg, GWLP_USERDATA)));
    const auto currentPos =
        int32_t(SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_GETPOS, 0, 0));

    if (data->dialogType == DIALOG_TYPE_THRESHOLD) {
      setValue(data->config->activationThreshold, currentPos);
      sliderMessage = L"Activation threshold: ";
    } else {
      setValue(data->config->digitalVibranceLevel, currentPos);
      sliderMessage = L"Select digital vibrance level: ";

      if (data->config->alwaysVibranceLevel.load(std::memory_order_relaxed)) {
        GPU::NvidiaSetDigitalVibrance(currentPos);
      }
    }

    UpdateSliderValueText(hDlg, currentPos, sliderMessage.data());
    break;
  }

  case SB_ENDSCROLL: {
    ((SharedWindowData *)(GetWindowLongPtr(hDlg, GWLP_USERDATA)))
        ->config->WriteConfig();
    break;
  }

  case WM_CLOSE: {
    data = ((SharedWindowData *)(GetWindowLongPtr(hDlg, GWLP_USERDATA)));
    data->lastWindow = nullptr;
    EndDialog(hDlg, INT_PTR(wParam));
    return TRUE;
  }
  }
  return FALSE;
}

void ShowCustomDialog(HWND hWnd, DLGPROC DialogProc, SharedWindowData *data) {
  // If the last window is still valid, bring it to the foreground
  // Do not allow multiple instances of the same dialog
  if (IsWindow(data->lastWindow)) {
    SetForegroundWindow(data->lastWindow);
    FLASHWINFO fwi;
    fwi.cbSize = sizeof(fwi);
    fwi.hwnd = data->lastWindow;
    fwi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
    fwi.uCount = 0;
    fwi.dwTimeout = 0;
    FlashWindowEx(&fwi);
    return;
  }

  auto hInstance = GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
  HWND hDlg = CreateDialogParam(HINSTANCE(hInstance),
                                MAKEINTRESOURCE(IDD_CUSTOM_DIALOG), hWnd,
                                DialogProc, (LPARAM)data);

  if (hDlg) {
    RECT screenRect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

    RECT rcDlg;
    GetWindowRect(hDlg, &rcDlg);

    const int dialogWidth = rcDlg.right - rcDlg.left;
    const int dialogHeight = rcDlg.bottom - rcDlg.top;
    const int dialogPosX = screenRect.right - dialogWidth;
    const int dialogPosY = screenRect.bottom - dialogHeight;

    SetWindowPos(hDlg, HWND_TOP, dialogPosX, dialogPosY, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER);
    ShowWindow(hDlg, SW_NORMAL);
  } else {
    MessageBox(nullptr,
               L"Failed to create a custom dialog near the system tray",
               L"Error", MB_ICONERROR | MB_OK);
  }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  SharedWindowData *data = nullptr;

  // Function to toggle option
  auto ToggleOption = [&data, &hWnd](auto &option, int item, auto func) {
    auto opt = !option.load(std::memory_order_relaxed);
    func(opt);
    option.store(opt, std::memory_order_relaxed);
    data->config->WriteConfig();
    CheckMenuItem((HMENU)hWnd, item, opt ? MF_CHECKED : MF_UNCHECKED);
  };

  switch (message) {
  case WM_APP + 1:
    if (lParam == WM_RBUTTONUP) {
      data = (SharedWindowData *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      ShowContextMenu(hWnd, data);
    }
    break;
  case WM_CREATE: {
    auto pcs = (LPCREATESTRUCT)lParam;
    data = ((SharedWindowData *)pcs->lpCreateParams);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);
    break;
  }
  case WM_COMMAND:
    data = (SharedWindowData *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    switch (LOWORD(wParam)) {
    case ACTIVATION_THRESHOLD_SETTING:
      data->dialogType = DIALOG_TYPE_THRESHOLD;
      ShowCustomDialog(hWnd, DialogProc, data);
      break;
    case DIGITAL_VIBRANCE_SETTING:
      data->dialogType = DIALOG_TYPE_DIGITAL_VIBRANCE;
      ShowCustomDialog(hWnd, DialogProc, data);
      break;
    case AUTOSTART:
      ToggleOption(data->config->autoStartup, AUTOSTART,
                   Utils::ManageAutoStartup);
      break;
    case NVIDIA_PERFORMANCE_MODE:
      ToggleOption(data->config->nvidiaPerformanceMode, NVIDIA_PERFORMANCE_MODE,
                   GPU::ToggleNvidiaPerformanceMode);
      break;
    case NVIDIA_ANSEL:
      ToggleOption(data->config->nvidiaAnsel, NVIDIA_ANSEL,
                   GPU::ToggleNvidiaAnsel);
      break;
    case NVIDIA_VSYNC:
      ToggleOption(data->config->nvidiaVsync, NVIDIA_VSYNC,
                   GPU::ToggleNvidiaVsync);
      break;
    case NVIDIA_LOW_LATENCY_MODE:
      ToggleOption(data->config->nvidiaLowLatencyMode, NVIDIA_LOW_LATENCY_MODE,
                   GPU::ToggleNvidiaLowLatencyMode);
      break;
    case NVIDIA_ULTRA_LOW_LATENCY_MODE:
      ToggleOption(data->config->nvidiaUltraLowLatencyMode,
                   NVIDIA_ULTRA_LOW_LATENCY_MODE,
                   GPU::ToggleNvidiaUltraLowLatencyMode);
      break;
    case ALWAYS_CPU_PERFORMANCE_MODE:
      ToggleOption(data->config->alwaysCPUPerformanceMode,
                   ALWAYS_CPU_PERFORMANCE_MODE, PowerManager::TogglePowerPlan);
      break;
    case ALWAYS_NVIDIA_PERFORMANCE_MODE:
      data->config->nvidiaPerformanceMode.store(true,
                                                std::memory_order_relaxed);
      ToggleOption(data->config->alwaysNvidiaPerformanceMode,
                   ALWAYS_NVIDIA_PERFORMANCE_MODE,
                   GPU::ToggleNvidiaPerformanceMode);
      break;
    case ALWAYS_VIBRANCE_LEVEL:
      ToggleOption(data->config->alwaysVibranceLevel, ALWAYS_VIBRANCE_LEVEL,
                   [](bool) {});
      break;
    case PLAY_ACTIVATION_SOUND:
      ToggleOption(data->config->playActivationSound, PLAY_ACTIVATION_SOUND,
                   [](bool) {});
      break;
    case EXIT_APP:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    break;
  case WM_CLOSE:
    DestroyWindow(hWnd);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}
