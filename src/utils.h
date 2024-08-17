#pragma once

#include <Windows.h>
#include <shlobj.h>
#include <string>
#include "constants.h"
#include "resources.h"

class ModuleHandle {
public:
  explicit ModuleHandle() noexcept : handle(nullptr) {}
  explicit ModuleHandle(const wchar_t *dllPath)
      : handle(LoadLibrary(dllPath)) {}
  ModuleHandle(const ModuleHandle &other) = delete;
  ModuleHandle &operator=(const ModuleHandle &other) = delete;

  ModuleHandle(ModuleHandle &&other) noexcept : handle(other.handle) {
    other.handle = nullptr;
  }

  ModuleHandle &operator=(ModuleHandle &&other) noexcept {
    if (this != &other) {
      if (handle)
        FreeLibrary(handle);
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }

  ~ModuleHandle() {
    if (handle) {
      FreeLibrary(handle);
    }
  }

  [[nodiscard]] HMODULE get() const { return handle; }

private:
  HMODULE handle;
};

namespace Utils {
  std::wstring GetAppDataPath();
  void PlayBubbleSound();

  bool IsFullScreen(HWND window);
  bool IsD3D();

  bool CheckAdministratorToken(PSID *ppAdministratorsGroup,
                               BOOL *pfIsRunAsAdmin);
  bool IsAdministrator();
  void ManageAutoStartup(bool autostartup);
  bool AddToStartup(const std::wstring &programPath);
  bool RemoveFromStartup();
  std::wstring GetExecutablePath();
}; // namespace Utils
