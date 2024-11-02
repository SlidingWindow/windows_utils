/*Utilitiy functions for Windows
* C++ utility functions to query currently running processes and walk the PEB structure
* Twitter: https://x.com/kapil_khot
* Blog: https://slidingwindow0xff.com
*/

#ifndef BUILD_WINDOWS
#define BUILD_WINDOWS
#endif // !BUILD_WINDOWS

#ifndef NTDLL_LIB
#pragma comment(lib, "ntdll")
#endif // !NTDLL_LIB

/* TO DO: Fix this later. SHould be able to use native API NtOpenProcess() from ntoskrnl.exe. Or it's not advisable?
#ifndef NTOSKERNL_LIB
#pragma comment(lib, "ntoskrnlx86")
#pragma comment(lib, "ntoskrnlx64")
#endif //!NTOSKERNL_LIB
*/

#include <windows.h>
#include <iostream>
#include <psapi.h>
#include <winternl.h>
//#include "ntddk.h"
//#include "ntifs.h"
#include <array>
#include <vector>

#define EXIT_CODE_ERROR 1
#define EXIT_CODE_SUCCESS 0

extern "C" void __stdcall RtlGetVersion(OSVERSIONINFOW * osverinfo);


class WinNativeAPIS
{
private:
	HANDLE hProcessHandle;
	std::vector<int> vProcessIds;

public:
	int getProcessInfo(int iProcessID) {
		PROCESS_BASIC_INFORMATION* sProcBasicInfo = new PROCESS_BASIC_INFORMATION(); // Allocate memory for sProcBasicInfo
		PEB* sPebBaseAddress = new PEB();
		RTL_USER_PROCESS_PARAMETERS* sProcessParameters = new RTL_USER_PROCESS_PARAMETERS();
		size_t bytesRead;
		int statusCode;
		size_t bytesOut;
		wchar_t wcBuffer[1024];
		//iProcessID = GetCurrentProcessId();
		hProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iProcessID); // ALL ACCESS or can try PROCESS_QUERY_INFORMATION | PROCESS_VM_READ

		if (!hProcessHandle) {
			std::cout << "\n[+]Error obtaining a handle to Process ID " << iProcessID << ":  " << GetLastError() << "\n";
			return EXIT_CODE_ERROR;
		}

		std::cout << "\n[+]Successfully obtained a handle to Process ID: " << iProcessID;

		if (NtQueryInformationProcess(hProcessHandle, ProcessBasicInformation, sProcBasicInfo, sizeof(PROCESS_BASIC_INFORMATION), nullptr) != 0) {
			std::cout << "[-]NtQueryInformationProcess() failed: " << GetLastError() << "\n";
			CloseHandle(hProcessHandle);
			return EXIT_CODE_ERROR;
		}

		std::cout << "\n\t[+]NtQueryInformationProcess() call succeeded: " << "\n";
		std::cout << "\t[+]Process ID: " << sProcBasicInfo->UniqueProcessId << "\n";
		std::cout << "\t[+]PEB Base Address: " << sProcBasicInfo->PebBaseAddress << "\n"; 
		// The following  works for current process. Just need to find a way to print the string instead of memory address.
		// TO DO: Check if it's possible to make it work for other processes on the system.
		//std::cout << "\t[+]sProcBasicInfo->PebBaseAddress->ProcessParameters->ImagePathName.Buffer: " << sProcBasicInfo->PebBaseAddress->ProcessParameters->ImagePathName.Buffer << "\n";

		// Read PEB Address from the target process
		if (!ReadProcessMemory(hProcessHandle, sProcBasicInfo->PebBaseAddress, sPebBaseAddress, sizeof(PEB), nullptr)) {
			std::cout << "\t[-]Error reading PebBaseAddress: " << GetLastError() << "\n";
			CloseHandle(hProcessHandle);
			return EXIT_CODE_ERROR;
		}

		// Get PEB->ProcessParameters
		if (!ReadProcessMemory(hProcessHandle, sPebBaseAddress->ProcessParameters, sProcessParameters, sizeof(RTL_USER_PROCESS_PARAMETERS), nullptr)) {
			std::cout << "\t[+]Error reading ProcessParameters structure: " << GetLastError() << "\n";
			CloseHandle(hProcessHandle);
			return EXIT_CODE_ERROR;
		}

		if (!ReadProcessMemory(hProcessHandle, sProcessParameters->ImagePathName.Buffer, wcBuffer, sizeof(wcBuffer), nullptr)) {
			std::cout << "\t[-]Error reading ImagePathName from ProcessParameters structure: " << GetLastError() << "\n";
			return EXIT_CODE_ERROR;
		}

		std::wcout << L"\t[+]sProcBasicInfo->PebBaseAddress->ProcessParameters->ImagePathName.Buffer: " << wcBuffer << "\n";
	}

};


class WinProcessAPIs
{
private:
	// Zero out the structure memory
	// STARTUPINFO lpSI;
	// ZeroMemory(&lpSI, 0, sizeof(lpSI));
	// lpSI.cb = sizeof(lpSI);
	STARTUPINFOW sStartupInfo = { sizeof(sStartupInfo) }; // One liner structure object initialization with zeroed out memory (or struct members) instead of the three statements above.
	PROCESS_INFORMATION sProcessInfo;
	DWORD dwcbNeeded, dwProcessCount;
	bool bcreateProcess;

public:
	void createProcess(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, 
		DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory)
	{
		bool bcreateProcess = CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, &sStartupInfo, &sProcessInfo);

		if (bcreateProcess) {
			std::cout << "\n[+]Process Created!" << "PID: " << sProcessInfo.dwProcessId << " Thread ID: " << sProcessInfo.dwThreadId;
			CloseHandle(sProcessInfo.hProcess);
			CloseHandle(sProcessInfo.hThread);
		}
		else {
			std::cout << "[+]Error creating process..." << GetLastError() << "\n";
		}
	}

	int enumProcesses(std::vector<int>& vProcessIdsList)
	{
		// To Do: Use dynamic arrays (std::vector) since we don't know how many processes will be there on the target machine.
		DWORD dwProcessIdsList[4096]; 

		if (!EnumProcesses(dwProcessIdsList, sizeof(dwProcessIdsList), &dwcbNeeded))
		{
			std::cout << "[+]Error while calling EnumProcesses() API" << GetLastError() << "\n";
			return EXIT_CODE_ERROR;
		}

		dwProcessCount = dwcbNeeded / sizeof(DWORD);
		std::cout << "[+]Number of processes running: " << dwProcessCount << "\n";
		
		for (int i = 0; i < dwProcessCount; i++) {
			vProcessIdsList.push_back(dwProcessIdsList[i]);
		}

		return EXIT_CODE_SUCCESS;
	}

	void getProcessInfo(int ipId)
	{
		HANDLE hOpenProcess = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, ipId);
		char processImageFileName[MAX_PATH];

		if (hOpenProcess)
		{
			std::cout << "\t[+]Opened a handle 0x" << hOpenProcess << " to PID: " << ipId << "\n";
			GetProcessImageFileName(hOpenProcess, processImageFileName, MAX_PATH);
			std::cout << "\t[+]Process image filename full path: " << processImageFileName << "\n";
			CloseHandle(hOpenProcess);
		}
		else 
			std::cout << "\t[+]Error opening a handle to PID " << ipId << ": " << GetLastError() << "\n";
	}

};


class OSVersionInfo
{
private:
	OSVERSIONINFOA osVerInfo = { sizeof(osVerInfo) }; // Set the first structure member, dwOSVersionInfoSize. Other members will be set by the called API.

public:
	template <typename T> void PrintOSVerInfo(T* osVerInfoStruct)
	{
		std::cout << "\t[+]Retrieved OS version: " << osVerInfoStruct->dwMajorVersion << "." << (int)osVerInfoStruct->dwMinorVersion << "." << (int)osVerInfoStruct->dwBuildNumber << "\n";
	}

	void GetOSVerInfo()
	{
		std::cout << "[+]Trying an NTDLL API RtlGetVersion()\n";
		RtlGetVersion(reinterpret_cast<OSVERSIONINFOW*>(&osVerInfo)); // Stores the OS version info in osVerInfo structure.
		PrintOSVerInfo(&osVerInfo);

		std::cout << "[+]Trying GetVersionEx() API to get the masked/fake version\n";
		GetVersionEx(&osVerInfo);
		PrintOSVerInfo(&osVerInfo);
	}
};


int main()
{
	std::cout << "\n[+]Trying different methods to get current OS version info.\n";
	OSVersionInfo osInfo;
	osInfo.GetOSVerInfo();

	// To Do: Use dynamic arrays (std::vector) since we don't know how many process will be there on the target machine.
	std::vector<int> vProcessIdsList; 

	WinProcessAPIs winProcess;
	WCHAR lpAppName[] = L"notepad";
	//winProcess.createProcess(nullptr, lpAppName, nullptr, nullptr, FALSE, HIGH_PRIORITY_CLASS, nullptr, nullptr);

	// TO DO: This reads out of bounds. need to use std::vector
	winProcess.enumProcesses(vProcessIdsList); 
	std::cout << "[+]Size of vProcessIdsList: " << vProcessIdsList.size() << "\n";
	//winProcess.getProcessInfo(7356);

	// Some process and thread related stuff using Native APIs
	WinNativeAPIS winProcessInfo;

	for (size_t i = 0; i < vProcessIdsList.size(); i++) {
		winProcessInfo.getProcessInfo(vProcessIdsList[i]);
	}

	winProcessInfo.getProcessInfo(7840);
	return EXIT_CODE_SUCCESS;
}
