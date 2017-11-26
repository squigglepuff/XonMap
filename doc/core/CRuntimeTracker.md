**class CRuntimeTracker**
================

*description*
----------------
This class is a runtime tracker, it's job is to track statistics on the program as well as provide runtime information incase of a crash. In addition, the class handles any errors that the program see's as critical or fatal. This class tracks the following statistics:
- Time running (in CPU clocks [clock_t])
- Memory usage (in KiB)
- CPU usage (in % and load (if possible))
- Net IO traffic (in KiB)

These variables are all tracked in a global CVar list defined in this module. It is a simple data structure like so:
**struct**
```
miRuntime:clock_t
miMemUsage:double
miCPUUsage:float
miNetIn:double
miNetOut:double
mbShouldRun:bool // Default is TRUE
```

This class is also responsible for handling all signal catching. The system is NOT allowed to ignore ANY signal it catches. This class is responsible for catching signals and updating the system to react to them.

*API*
================
The class should expose the following API to the world:
```cpp
	typedef unsigned int Error_t

	Error_t Init(void)
		// This function initializes the system and begins tracking data.

	Error_t Reset(void)
		// This function resets all tracked data back to a sane state (i.e. 0).

	Error_t Update(void)
		// This function updates the tracking data. This function should be called with every "tick" of the server so the data is being updated in realtime.

	Error_t DumpStack(void)
		// This function will dump the stack of the program's runtime into the logging sub-system. This function will first construct a single line (with LF-CR breaks) before passing it to the logger.

	Error_t DumpStats(bool iReset = false)
		// This function dumps all the tracked statistics to the logging system. The function will first construct a neat table in a single line (with LF-CR breaks) before passing it to the logger.

	std::string ErrToQStr(Error_t iErr = Success) // OVERLOAD
	std::string ErrorToString(Error_t iErr = Success)
		// These functions translate an Error_t to a human-readable error/warning string.
```

*Defines*
----------------
| Name    | Value      |
|---------|------------|
| Success | 0x00000000 |

*Enumerations*
----------------
**ErrorCodes**
*[ 1 .. cerrno errors .. 255 ]*

| Enum                | Code       | Meaning               |
|---------------------|------------|-----------------------|
| Err_Sig_Hangup      | 0xC0000001 | Caught SIGHUP (1)     |
| Err_Sig_Quit        | 0xC0000002 | Caught SIGQUIT (3)    |
| Err_Sig_Illegal     | 0xC0000003 | Caught SIGILL (4)     |
| Err_Sig_Abort       | 0xC0000004 | Caught SIGABRT (6)    |
| Err_Sig_FloatExcept | 0xC0000005 | Caught SIGFPE (8)     |
| Err_Sig_Kill        | 0xC0000006 | Caught SIGKILL (9)    |
| Err_Sig_Bus         | 0xC0000007 | Caught SIGBUS (10)    |
| Err_Sig_SegFault    | 0xC0000008 | Caught SIGSEGV (11)   |
| Err_Sig_SysCall     | 0xC0000009 | Caught SIGSYS (12)    |
| Err_Sig_Pipe        | 0xC000000a | Caught SIGPIPE (13)   |
| Err_Sig_Alarm       | 0xC000000b | Caught SIGALRM (14)   |
| Err_Sig_Terminate   | 0xC000000c | Caught SIGTERM (15)   |
| Err_Sig_XCPU        | 0xC000000d | Caught SIGXCPU (24)   |
| Err_Sig_FSizeLimit  | 0xC000000e | Caught SIGXFSZ (25)   |
| Err_Sig_VirtAlarm   | 0xC000000f | Caught SIGVTALRM (26) |
| Err_Sig_ProfAlarm   | 0xC0000010 | Caught SIGPROF (27)   |

**Warnings**

| Enum            | Code       | Meaning               									 |
|-----------------|------------|---------------------------------------------------------|
| Warn_LowMemory  | 0xA0000001 | System is low on memory 								 |
| Warn_LowFD      | 0xA0000002 | System is low on file descriptors  					 |
| Warn_LowThreads | 0xA0000003 | System is low on available threads 					 |
| Warn_LowSpace   | 0xA0000004 | System is low on HDD space 							 |
| Warn_INetDown   | 0xA0000005 | System doesn't have an network connection               |
| Warn_INetLimit  | 0xA0000006 | System seems to have a LAN connection, but no internet. |

*Data Structures*
----------------

**[Global] GlobalCVariables**
This data structure is used to track all of the runtime statistics.

| Data Type | Member Name  | Usage                                                  |
|-----------|--------------|--------------------------------------------------------|
| clock_t   | mRuntime     | Time running (in CPU clocks).                          |
| float     | mMemSysTotal | Total amount of memory the host has (in MiB).          |
| float     | mMemSysUsed  | Total amount of used memory the host has (in MiB).     |
| float     | mMemSysFree  | Total amount of free memory the host has (in MiB).     |
| float     | mMemProcUsed | Total amount of used memory for this process (in MiB). |
| float     | mCPUSysUsage | Host system CPU usage (in %).                          |
| float     | mNetIn       | Incoming network traffic (in KiB).                     |
| float     | mNetOut      | Outgoing network traffic (in KiB).                     |
