# Windows Utility Functions
C++ utility functions to query currently running processes and walk the PEB structure

```
c:\Users\admin\Desktop>WinUtils.exe

[+]Trying different methods to get current OS version info.
[+]Trying an NTDLL API RtlGetVersion()
        [+]Retrieved OS version: 10.0.19045
[+]Trying GetVersionEx() API to get the masked/fake version
        [+]Retrieved OS version: 6.2.9200
[+]Number of processes running: 150
[+]Size of vProcessIdsList: 150

[+]Error obtaining a handle to Process ID 0:  87

[+]Error obtaining a handle to Process ID 4:  5
<OMITTED>

[+]Successfully obtained a handle to Process ID: 2636
        [+]NtQueryInformationProcess() call succeeded:
        [+]Process ID: 2636
        [+]PEB Base Address: 0000008663E78000
        [+]sProcBasicInfo->PebBaseAddress->ProcessParameters->ImagePathName.Buffer: C:\Windows\system32\sihost.exe

[+]Successfully obtained a handle to Process ID: 3492
        [+]NtQueryInformationProcess() call succeeded:
        [+]Process ID: 3492
        [+]PEB Base Address: 0000005BFF11E000
        [+]sProcBasicInfo->PebBaseAddress->ProcessParameters->ImagePathName.Buffer: C:\Windows\system32\svchost.exe

[+]Successfully obtained a handle to Process ID: 6048
        [+]NtQueryInformationProcess() call succeeded:
        [+]Process ID: 6048
        [+]PEB Base Address: 000000C84459C000
        [+]sProcBasicInfo->PebBaseAddress->ProcessParameters->ImagePathName.Buffer: C:\Windows\system32\svchost.exe

[+]Successfully obtained a handle to Process ID: 6260
        [+]NtQueryInformationProcess() call succeeded:
        [+]Process ID: 6260
        [+]PEB Base Address: 0000001FE91C6000
        [+]sProcBasicInfo->PebBaseAddress->ProcessParameters->ImagePathName.Buffer: C:\Windows\system32\taskhostw.exe

<OMITTED>
```
