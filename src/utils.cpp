#include "utils.h"

namespace Utils {
  void PlayBubbleSound() {
    PlaySound(MAKEINTRESOURCE(IDR_WAVE1), nullptr, SND_RESOURCE | SND_ASYNC);
  }

  std::wstring GetAppDataPath() {
    PWSTR ppszPath = nullptr;
    const HRESULT hr =
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &ppszPath);
    if (SUCCEEDED(hr)) {
      std::wstring appDataPathStr = ppszPath;
      appDataPathStr += L"\\PowerPlanManager";

      if (CreateDirectoryW(appDataPathStr.c_str(), nullptr) ||
          ERROR_ALREADY_EXISTS == GetLastError()) {
        CoTaskMemFree(ppszPath);
        return appDataPathStr;
      }
      CoTaskMemFree(ppszPath);
    }
    return L"";
  }

  bool IsFullScreen(HWND window) {
    RECT windowRect, desktopRect;
    GetWindowRect(window, &windowRect);
    GetWindowRect(GetDesktopWindow(), &desktopRect);
    return (windowRect.left == desktopRect.left &&
            windowRect.right == desktopRect.right &&
            windowRect.top == desktopRect.top &&
            windowRect.bottom == desktopRect.bottom);
  }

  bool IsD3D() {
    QUERY_USER_NOTIFICATION_STATE pquns = {};
    SHQueryUserNotificationState(&pquns);

    if (pquns == QUNS_RUNNING_D3D_FULL_SCREEN || pquns == QUNS_BUSY) {
      return true;
    }
    return false;
  }

  bool CheckAdministratorToken(PSID *ppAdministratorsGroup,
                               BOOL *pfIsRunAsAdmin) {
    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                  ppAdministratorsGroup)) {
      return false;
    }

    // Determine whether the SID of administrators group is enabled in
    // the primary access token of the process.
    if (!CheckTokenMembership(nullptr, *ppAdministratorsGroup,
                              pfIsRunAsAdmin)) {
      return false;
    }

    return true;
  }

  bool IsAdministrator() {
    BOOL IsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = nullptr;

    if (!CheckAdministratorToken(&pAdministratorsGroup, &IsAdmin)) {
      dwError = GetLastError();
    }

    if (pAdministratorsGroup) {
      FreeSid(pAdministratorsGroup);
    }

    if (ERROR_SUCCESS != dwError) {
      return false;
    }

    return IsAdmin;
  }

  void ManageAutoStartup(bool enableAutoStartup) {
    if (!IsAdministrator()) {
      auto szPath = GetExecutablePath();
      if (!szPath.empty()) {
        SHELLEXECUTEINFO sei = {sizeof(sei)};
        sei.lpVerb = L"runas";
        sei.lpFile = szPath.data();
        sei.hwnd = nullptr;
        sei.nShow = SW_HIDE;

        if (enableAutoStartup) {
          sei.lpParameters = L"/autostartup";
        } else {
          sei.lpParameters = L"/removeautostartup";
        }

        if (!ShellExecuteEx(&sei)) {
          const DWORD dwError = GetLastError();
          if (dwError == ERROR_CANCELLED) {
            MessageBox(nullptr, L"The user cancelled elevation", L"Error",
                       MB_OK);
          }
        }
      }
    } else {
      if (enableAutoStartup) {
        AddToStartup(GetExecutablePath());
      } else {
        RemoveFromStartup();
      }
    }
  }

  bool AddToStartup(const std::wstring &programPath) {
    HKEY hkey = nullptr;
    LONG result = RegOpenKeyEx(
        HKEY_CURRENT_USER, LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)",
        0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hkey);

    if (result != ERROR_SUCCESS) {
      return false;
    }

    auto *valueName = L"PowerPlanManager";
    result = RegSetValueEx(hkey, valueName, 0, REG_SZ,
                           (const BYTE *)(programPath.c_str()),
                           (programPath.size() + 1) * sizeof(wchar_t));

    RegCloseKey(hkey);

    return (result == ERROR_SUCCESS);
  }

  bool RemoveFromStartup() {
    HKEY hkey = nullptr;
    LONG result = RegOpenKeyEx(
        HKEY_CURRENT_USER, LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)",
        0, KEY_SET_VALUE | KEY_WOW64_64KEY, &hkey);

    if (result != ERROR_SUCCESS) {
      return false;
    }

    const wchar_t *valueName = L"PowerPlanManager";
    result = RegDeleteValue(hkey, valueName);

    RegCloseKey(hkey);

    return (result == ERROR_SUCCESS);
  }

  std::wstring GetExecutablePath() {
    constexpr uint32_t initialBufferSize = MAX_PATH;
    uint32_t bufferSize = initialBufferSize;
    std::wstring path;

    while (true) {
      path.resize(bufferSize, L'\0');
      const size_t size =
          GetModuleFileName(nullptr, path.data(), (DWORD)(bufferSize));

      if (size == 0) {
        return L"";
      } else if (size < bufferSize) {
        path.resize(size);
        path.shrink_to_fit();
        return path;
      } else {
        bufferSize *= 2;
      }
    }
  }
} // namespace Utils
