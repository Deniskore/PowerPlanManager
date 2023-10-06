#pragma once

#include <Windows.h>
#include <shlobj.h>
#include <string>
#include "constants.h"
#include "resources.h"

class ModuleHandle
{
public:
    explicit ModuleHandle() noexcept
        : handle(nullptr) {}
    explicit ModuleHandle(const wchar_t* dllPath)
        : handle(LoadLibrary(dllPath)) {}
    ModuleHandle(const ModuleHandle& other) = delete;
    ModuleHandle& operator=(const ModuleHandle& other) = delete;

    ModuleHandle(ModuleHandle&& other) noexcept
        : handle(other.handle)
    {
        other.handle = nullptr;
    }

    ModuleHandle& operator=(ModuleHandle&& other) noexcept
    {
        if (this != &other)
        {
            if (handle)
                FreeLibrary(handle);
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    ~ModuleHandle()
    {
        if (handle)
        {
            FreeLibrary(handle);
        }
    }

    [[nodiscard]] HMODULE get() const { return handle; }

private:
    HMODULE handle;
};

class Utils {
public:
  Utils() = delete;
  ~Utils() = delete;
  Utils(const Utils &) = delete;
  Utils(Utils &&) = delete;
  Utils &operator=(Utils &&) = delete;
  Utils &operator=(const Utils &) = delete;

  static std::wstring GetAppDataPath();
  static void PlayBubbleSound();

  static bool IsFullScreen(HWND window);
  static bool IsD3D();

  static bool CheckAdministratorToken(PSID *ppAdministratorsGroup,
                                      BOOL *pfIsRunAsAdmin);
  static bool IsAdministrator();
  static void ManageAutoStartup(bool autostartup);
  static bool AddToStartup(const std::wstring &programPath);
  static bool RemoveFromStartup();
  static std::wstring GetExecutablePath();
};
