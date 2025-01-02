// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <psapi.h>
#include <winnt.h>
#include <winternl.h>
#include <TlHelp32.h>
#include <Windows.h>
#include <string.h>
#include <vector>
#include <algorithm>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

std::vector<std::string> hiddenProcesses;
const int maxHiddenProcesses = 5;

typedef NTSTATUS(WINAPI* PNT_QUERY_SYSTEM_INFORMATION)(
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __inout PVOID SystemInformation,
    __in ULONG SystemInformationLength,
    __out_opt PULONG ReturnLength
    );

int stringToInt(const std::string& str) {
    int value = 0;
    for (char ch : str) {
        if (!isdigit(ch)) return -1; // Return -1 for invalid input
        value = value * 10 + (ch - '0');
    }
    return value;
}

PNT_QUERY_SYSTEM_INFORMATION origNtQuerySysInfo = (PNT_QUERY_SYSTEM_INFORMATION)GetProcAddress(GetModuleHandle("ntdll"), "NtQuerySystemInformation");

NTSTATUS WINAPI hookNtQuerySysInfo(
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __inout PVOID SystemInformation,
    __in ULONG SystemInformationLength,
    __out_opt PULONG ReturnLength
) {
    NTSTATUS status = origNtQuerySysInfo(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

    if (SystemProcessInformation == SystemInformationClass && STATUS_SUCCESS == status)
    {
        SYSTEM_PROCESS_INFORMATION* pCurrent = (SYSTEM_PROCESS_INFORMATION*)SystemInformation;

        while (pCurrent->NextEntryOffset != 0) {
            SYSTEM_PROCESS_INFORMATION* pNext = (SYSTEM_PROCESS_INFORMATION*)((PUCHAR)pCurrent + pCurrent->NextEntryOffset);

            std::string processName(pNext->ImageName.Buffer, pNext->ImageName.Buffer + pNext->ImageName.Length / sizeof(WCHAR));
            std::transform(processName.begin(), processName.end(), processName.begin(), ::tolower);

            if (std::find(hiddenProcesses.begin(), hiddenProcesses.end(), processName) != hiddenProcesses.end()) {
                pCurrent->NextEntryOffset += pNext->NextEntryOffset;
            }
            else {
                pCurrent = pNext;
            }
        }
    }
    return status;
}

void clearConsole() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count, cellCount;
    COORD homeCoords = { 0, 0 };

    if (hConsole == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', cellCount, homeCoords, &count)) return;

    if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count)) return;

    SetConsoleCursorPosition(hConsole, homeCoords);
}

void displayBanner(MODULEINFO& modInfo, uintptr_t hookAddress) {
    clearConsole();
    std::cout << "Task Hider v1 | by lo5r" << std::endl;
    std::cout << "\n\n";
    std::cout << "Base address: " << modInfo.lpBaseOfDll << std::endl;
    std::cout << "Successfully Hooked Taskmgr.exe: " << hookAddress << std::endl;
}

DWORD WINAPI main(HMODULE hModule) {
    AllocConsole();
    FILE* f;
    FILE* f2;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f2, "CONIN$", "r", stdin);

    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(0), &modInfo, sizeof(MODULEINFO));

    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)modInfo.lpBaseOfDll;
    IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)((BYTE*)modInfo.lpBaseOfDll + dosHeader->e_lfanew);
    IMAGE_OPTIONAL_HEADER optionalHeader = (IMAGE_OPTIONAL_HEADER)(ntHeader->OptionalHeader);
    IMAGE_IMPORT_DESCRIPTOR* importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)(modInfo.lpBaseOfDll) + optionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (importDescriptor->Characteristics) {
        if (strcmp("ntdll.dll", (char*)((BYTE*)modInfo.lpBaseOfDll + importDescriptor->Name)) == 0) {
            break;
        }
        importDescriptor++;
    }

    IMAGE_THUNK_DATA* tableEntry = (IMAGE_THUNK_DATA*)((BYTE*)modInfo.lpBaseOfDll + importDescriptor->OriginalFirstThunk);
    IMAGE_THUNK_DATA* IATEntry = (IMAGE_THUNK_DATA*)((BYTE*)modInfo.lpBaseOfDll + importDescriptor->FirstThunk);
    IMAGE_IMPORT_BY_NAME* funcName;

    while (!(tableEntry->u1.Ordinal & IMAGE_ORDINAL_FLAG) && tableEntry->u1.AddressOfData) {
        funcName = (IMAGE_IMPORT_BY_NAME*)((BYTE*)modInfo.lpBaseOfDll + tableEntry->u1.AddressOfData);
        if (strcmp("NtQuerySystemInformation", (char*)(funcName->Name)) == 0) {
            break;
        }
        tableEntry++;
        IATEntry++;
    }

    DWORD oldProt;
    VirtualProtect(&(IATEntry->u1.Function), sizeof(uintptr_t), PAGE_READWRITE, &oldProt);
    IATEntry->u1.Function = (uintptr_t)hookNtQuerySysInfo;
    VirtualProtect(&(IATEntry->u1.Function), sizeof(uintptr_t), oldProt, &oldProt);

    uintptr_t hookAddress = (uintptr_t)hookNtQuerySysInfo;
    displayBanner(modInfo, hookAddress);

    while (true) {
        std::cout << "\n\n";
        std::cout << "[1] Hide Process" << std::endl;
        std::cout << "[2] Hidden Processes" << std::endl;
        std::cout << "[H] Hide Console" << std::endl;
        std::cout << "[X] Close (Will Unhook)" << std::endl;
        std::cout << "\nEnter your choice: ";
        std::string choice;
        std::cin >> choice;

        if (choice == "1") {
            displayBanner(modInfo, hookAddress);
            std::cout << "\n[INFO] You can only Hide 5 processes at a time." << std::endl;
            std::cout << "\n\nName of process to hide: ";
            std::string tmp;
            std::cin >> tmp;
            std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

            if (hiddenProcesses.size() >= maxHiddenProcesses) {
                std::cout << "Max processes reached. Please unhide a process to hide this one." << std::endl;
            }
            else {
                hiddenProcesses.push_back(tmp);
                std::cout << "Process " << tmp << " is now hidden." << std::endl;
            }
        }
        else if (choice == "H" || choice == "h") {
            ShowWindow(GetConsoleWindow(), SW_HIDE);
        }
        else if (choice == "2") {
            displayBanner(modInfo, hookAddress);

            std::cout << "\n[M] Menu" << std::endl;
            for (size_t i = 0; i < hiddenProcesses.size(); ++i) {
                std::cout << "[" << (i + 1) << "] " << hiddenProcesses[i] << std::endl;
            }

            std::cout << "\nChoose process to unhide: ";
            std::string selection;
            std::cin >> selection;

            if (selection == "M" || selection == "m") {
                continue;
            }

            int index = stringToInt(selection) - 1;
            if (index >= 0 && index < hiddenProcesses.size()) {
                std::cout << "Unhiding process: " << hiddenProcesses[index] << std::endl;
                hiddenProcesses.erase(hiddenProcesses.begin() + index);
            }
            else {
                std::cout << "Invalid selection. Returning to menu..." << std::endl;
            }
        }
        else if (choice == "X" || choice == "x") {
            VirtualProtect(&(IATEntry->u1.Function), sizeof(uintptr_t), PAGE_READWRITE, &oldProt);
            IATEntry->u1.Function = (uintptr_t)origNtQuerySysInfo;
            VirtualProtect(&(IATEntry->u1.Function), sizeof(uintptr_t), oldProt, &oldProt);

            fclose(f);
            fclose(f2);
            FreeConsole();
            FreeLibraryAndExitThread(hModule, 0);
            return 0;
        }
        else {
            std::cout << "Invalid choice. Returning to menu..." << std::endl;
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)main, hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
