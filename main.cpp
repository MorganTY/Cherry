#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <sddl.h>
#include <string>
#include <Lmcons.h>
#include <strsafe.h>
#include <vector>

using namespace std;

// Function to delete a file permanently
void deleteFile(const std::wstring& path) {
    if (DeleteFile(path.c_str())) {
        std::wcout << L"File deleted: " << path << std::endl;
    }
    else {
        std::wcerr << L"Failed to delete file: " << path << L" Error: " << GetLastError() << std::endl;
    }
}
void folderDelete(const std::wstring& folderPath) {
    std::wstring searchPath = folderPath + L"\\*";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::wcerr << "Invalid directory path: " << folderPath << std::endl;
        return;
    }

    do {
        const std::wstring fileOrDirName = findFileData.cFileName;

        if (fileOrDirName != L"." && fileOrDirName != L"..") {
            std::wstring fullPath = folderPath + L"\\" + fileOrDirName;

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // Recursive call for directories
                folderDelete(fullPath);
            }
            else {
                // Delete the file
                if (!DeleteFile(fullPath.c_str())) {
                    std::wcerr << L"Failed to delete file: " << fullPath << std::endl;
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    // Now delete the folder itself
    if (!RemoveDirectory(folderPath.c_str())) {
        std::wcerr << L"Failed to delete directory: " << folderPath << std::endl;
    }
    else {
        std::wcout << L"Successfully Deleted Directory: " << folderPath << std::endl;
    }
}
// Function for HKEY_USERS
std::wstring EnumerateHKeyUsersWithPrefix(const wchar_t* prefix, const wchar_t* ignoreSubstring) {
    std::wstring resultString;
    HKEY hKey;
    LONG result;

    // Open the HKEY_USERS key
    result = RegOpenKeyEx(HKEY_USERS, L"", 0, KEY_READ | KEY_WOW64_64KEY, &hKey);
    if (result != ERROR_SUCCESS) {
        resultString = L"Failed to open HKEY_USERS.";
        return resultString;
    }

    // Enumerate subkeys under HKEY_USERS
    WCHAR subKeyName[256];
    DWORD subKeyNameSize = ARRAYSIZE(subKeyName);
    DWORD index = 0;

    while (true) {
        result = RegEnumKeyEx(hKey, index++, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL);
        if (result == ERROR_SUCCESS) {
            // Check if the subkey name starts with the specified prefix and does not contain the ignore substring
            if (wcsncmp(subKeyName, prefix, wcslen(prefix)) == 0 && !wcsstr(subKeyName, ignoreSubstring)) {
                resultString += subKeyName;
                resultString += L"";
            }
            subKeyNameSize = ARRAYSIZE(subKeyName); // Reset size for next enumeration
        }
        else if (result == ERROR_NO_MORE_ITEMS) {
            break; // No more subkeys
        }
        else {
            resultString = L"Failed to enumerate subkeys under HKEY_USERS.";
            break;
        }
    }

    // Close HKEY_USERS
    RegCloseKey(hKey);

    return resultString;
}

// Function to get the current username
std::wstring getUsername() {
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserName(username, &size)) {
        return std::wstring(username);
    }
    else {
        std::wcerr << L"Failed to get username. Error: " << GetLastError() << std::endl;
        return L"";
    }
}

// Function to delete registry key recursively
bool DeleteRegistryKeyRecursively(HKEY hKeyRoot, const std::wstring& subKey) {
    HKEY hKey;
    LONG result;

    // Open the registry key
    result = RegOpenKeyEx(hKeyRoot, subKey.c_str(), 0, KEY_READ | KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open registry key: " << subKey << L" Error: " << result << std::endl;
        return false;
    }

    // Enumerate all subkeys and delete them
    DWORD subKeyIndex = 0;
    wchar_t subKeyName[256];
    DWORD subKeyNameLen = 256;

    while (result == ERROR_SUCCESS) {
        subKeyNameLen = 256; // Reset the length for the next call
        result = RegEnumKeyEx(hKey, subKeyIndex, subKeyName, &subKeyNameLen, nullptr, nullptr, nullptr, nullptr);
        if (result == ERROR_SUCCESS) {
            // Recursively delete the subkey
            if (!DeleteRegistryKeyRecursively(hKey, subKeyName)) {
                RegCloseKey(hKey);
                return false;
            }
            subKeyIndex++;
        }
        else if (result != ERROR_NO_MORE_ITEMS) {
            std::wcerr << L"Failed to enumerate subkeys: " << subKey << L" Error: " << result << std::endl;
            RegCloseKey(hKey);
            return false;
        }
    }

    // Close the registry key
    RegCloseKey(hKey);

    // Now delete the empty key
    result = RegDeleteKeyEx(hKeyRoot, subKey.c_str(), KEY_WOW64_64KEY, 0);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to delete registry key: " << subKey << L" Error: " << result << std::endl;
        return false;
    }

    return true;
}


// Main function to demonstrate file and registry key deletion
int main() {

    //Main Var
    std::string input;
    std::wstring username = getUsername();
    std::wcout << L"Your Current Account Name: " << username << L"\n" << endl;

    //Unleashed Var
    std::wstring roamingDirectoryUnleashed = L"C:\\Users\\" + username + L"\\AppData\\Roaming\\Unleashed";
    std::wstring acFileCrash = L"C:\\Users\\" + username + L"\\AppData\\Local\\Packages\\StudioWildcard.4558480580BB9_1w2mm55455e38\\Ac\\CRASH_LOG.txt";
    std::wstring acFolderUnleashed = L"C:\\Users\\" + username + L"\\AppData\\Local\\Packages\\StudioWildcard.4558480580BB9_1w2mm55455e38\\Ac\\UNLEASHED";

    //Headshot Var
    std::wstring hsLogFolder = L"C:\\Users\\" + username + L"\\AppData\\Local\\Packages\\StudioWildcard.4558480580BB9_1w2mm55455e38\\AC\\Temp\\HeadshotLogs";
    std::wstring hsConfigFolder = L"C:\\Users\\" + username + L"\\AppData\\Local\\Packages\\StudioWildcard.4558480580BB9_1w2mm55455e38\\AC\\Temp\\HeadshotConfigs";
    std::wstring winTemp = L"C:\\Users\\" + username + L"\\AppData\\Local\\Temp";
    std::wstring registryAppSwitched = L"";

    //Shared Val
    std::wstring winFileShell = L"C:\\Users\\" + username + L"\\AppData\\Local\\Microsoft\\Windows\\Shell"; // Find??
    std::wstring winFileRecent = L"C:\\Users\\" + username + L"\\AppData\\Roaming\\Microsoft\\Windows\\Recent"; // 
    std::wstring hkeyMui = L"HKEY_CURRENT_USER\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache"; //Further Work

    std::wstring currentHKEYUser = EnumerateHKeyUsersWithPrefix(L"S-1-5-21-", L"Classes"); // prefix & substring remove
    std::wstring assistantStore = L"\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Compatibility Assistant\\Store";
    std::wstring compatibilityStoreKey = currentHKEYUser + assistantStore;

    std::wstring basicSwitched = L"\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FeatureUsage\\AppSwitched";
    std::wstring switchedStoreKey = currentHKEYUser + basicSwitched;

    bool running = true;

    while (running) {
        std::wcout << "Please Ensure Running as ADMIN\nPlease Ensure Ark is Closed\n";
        std::wcout << "\nChoose an Option \n[1] Unleashed\n[2] Headshot\n[3] Goldfish\n[4] Primal\n[5] Disable User Assist\n[6] Exit\n ";
        std::cin >> input;

        switch (input[0]) {
        case '1':
            if (input == "1") {
                std::wcout << "You chose Unleashed.\n";
                std::wcout << "Removing Files for Unleashed\n";
                deleteFile(acFileCrash);
                folderDelete(roamingDirectoryUnleashed);
                folderDelete(acFolderUnleashed);
                folderDelete(winFileShell);
                folderDelete(winFileRecent);
                DeleteRegistryKeyRecursively(HKEY_USERS, compatibilityStoreKey);
                DeleteRegistryKeyRecursively(HKEY_CURRENT_USER, hkeyMui);

                /*
                recursive registry removal for app
                if (DeleteRegistryKeyRecursively(HKEY_USERS, compatibilityStoreKey)) {
                    std::wcout << L"Registry key deleted successfully." << std::endl;
                }
                else {
                    std::wcout << L"Failed to delete registry key." << std::endl;
                }
                if (DeleteRegistryKeyRecursively(HKEY_CURRENT_USER, hkeyMui)) {
                    std::wcout << L"Registry key deleted successfully." << std::endl;
                }
                else {
                    std::wcout << L"Failed to delete registry key." << std::endl;
                }

                */


                break;
            }
            // fall through if not matched

        case '2':
            if (input == "2") {
                std::wcout << "You chose Headshot.\n";

                std::wcout << "Please Close Ark or some Files will not remove!!\n";
                folderDelete(hsLogFolder);
                folderDelete(hsConfigFolder);
                folderDelete(winTemp);


                DeleteRegistryKeyRecursively(HKEY_USERS, compatibilityStoreKey);
                DeleteRegistryKeyRecursively(HKEY_CURRENT_USER, hkeyMui);
                DeleteRegistryKeyRecursively(HKEY_USERS, switchedStoreKey);
                /*
                recursive registry removal for app
                if (DeleteRegistryKeyRecursively(HKEY_USERS, switchedStoreKey)) {
                    std::wcout << L"Registry key deleted successfully." << std::endl;
                }
                else {
                    std::wcout << L"Failed to delete registry key." << std::endl;
                }
                */
                break;
            }
            // fall through if not matched

        case '3':
            if (input == "3") {
                std::wcout << "You chose Goldfish.\n";
                break;
            }
            // fall through if not matched

        case '4':
            if (input == "4") {
                std::wcout << "You chose Primal.\n";
                break;
            }
            // fall through if not matched

        case '5':
            if (input == "5") {
                std::wcout << "How to disable User Assist\n";

                break;
            }
        case '6':
            if (input == "6") {
                std::wcout << "Exiting program.\n";
                running = false;
                break;
            }
        default:
            std::wcout << "Invalid command.\n";
            break;
        }
    }
    return 0;
}
