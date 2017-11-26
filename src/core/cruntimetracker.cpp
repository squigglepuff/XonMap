#include "core/cruntimetracker.h"

std::unique_ptr<GlobalCVariables> g_pCVars;
std::unique_ptr<CRuntimeTracker> g_CrtTracker{new CRuntimeTracker};

// These are local to this module only!
#if defined(Q_OS_WINDOWS)
static PDH_HQUERY gCpuQuery;
static PDH_HCOUNTER gCpuTotal;
#endif //#if (Q_OS_WINDOWS).

CRuntimeTracker::CRuntimeTracker()
{
    // Intentionally left blank.
}

CRuntimeTracker::~CRuntimeTracker()
{
    // Intentionally left blank.
}

Error_t CRuntimeTracker::Init()
{
    Error_t iRtn = SUCCESS;

    // Attempt to reset the pointer back to a sane state.
    g_pCVars.reset(new GlobalCVariables);

    // WINDOWS ONLY! Setup the PDH and Symbol stuff.
#if defined(Q_OS_WINDOWS)
    PdhOpenQuery(NULL, NULL, &gCpuQuery);
    PdhAddEnglishCounter(gCpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &gCpuTotal);

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
#endif //#if defined(Q_OS_WINDOWS).

    if (nullptr == g_pCVars)
    {
        iRtn = Err_Ptr_AllocFail;
    }
    else
    {
        iRtn = Update();
    }

    return iRtn;
}

Error_t CRuntimeTracker::Update()
{
    Error_t iRtn = SUCCESS;
#if defined(Q_OS_WINDOWS)
    // Grab system memory statistics.
    MEMORYSTATUSEX lMemInfo;
    lMemInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (0 != GlobalMemoryStatusEx(&lMemInfo))
    {
        DWORDLONG iFreeVirtMem = lMemInfo.ullAvailPageFile;
        DWORDLONG iTotalVirtMem = lMemInfo.ullTotalPageFile;
        DWORDLONG iUsedVirtMem = iTotalVirtMem - iFreeVirtMem;

        // Update the CVar.
        g_pCVars->mMemSysTotal = static_cast<float>(iTotalVirtMem) / 1024 / 1024;
        g_pCVars->mMemSysFree = static_cast<float>(iFreeVirtMem) / 1024 / 1024;
        g_pCVars->mMemSysUsed = static_cast<float>(iUsedVirtMem) / 1024 / 1024;

        // Grab the process memory statistics.
        PROCESS_MEMORY_COUNTERS lProcMemCnt;
        if (0 != GetProcessMemoryInfo(GetCurrentProcess(), &lProcMemCnt, sizeof(lProcMemCnt)))
        {
            SIZE_T iProcMemUsage = lProcMemCnt.PagefileUsage;

            // Update the CVar.
            g_pCVars->mMemProcUsed = static_cast<float>(iProcMemUsage) / 1024 / 1024;

            // Grab the CPU stats for the system.
            PDH_FMT_COUNTERVALUE lCountVal;

            PDH_STATUS lStatus = PdhCollectQueryData(gCpuQuery);
            if (ERROR_SUCCESS == lStatus)
            {
                lStatus = PdhGetFormattedCounterValue(gCpuTotal, PDH_FMT_DOUBLE, NULL, &lCountVal);
                if (ERROR_SUCCESS == lStatus)
                {
                    // Update the CVar.
                    g_pCVars->mCPUSysUsage = static_cast<float>(lCountVal.doubleValue);

                    // If needed, we can setup any network statistics right here.
                }
                else if (lStatus == PDH_INVALID_ARGUMENT)
                {
                    fprintf(stderr, "ERR: A parameter is not valid or is incorrectly formatted.\n");
                    iRtn = GetLastError();
                }
                else if (lStatus == PDH_INVALID_DATA)
                {
                    fprintf(stderr, "ERR: The specified counter does not contain valid data or a successful status code.\n");
                    iRtn = SUCCESS;
                }
                else if (lStatus == PDH_INVALID_HANDLE)
                {
                    fprintf(stderr, "ERR: The counter handle is not valid.\n");
                    iRtn = GetLastError();
                }
                else
                {
                    iRtn = GetLastError();
                }
            }
            else if (lStatus == PDH_INVALID_ARGUMENT)
            {
                fprintf(stderr, "ERR: A parameter is not valid or is incorrectly formatted.\n");
                iRtn = GetLastError();
            }
            else if (lStatus == PDH_INVALID_DATA)
            {
                fprintf(stderr, "ERR: The specified counter does not contain valid data or a successful status code.\n");
                iRtn = SUCCESS;
            }
            else if (lStatus == PDH_INVALID_HANDLE)
            {
                fprintf(stderr, "ERR: The counter handle is not valid.\n");
                iRtn = GetLastError();
            }
            else
            {
                iRtn = GetLastError();
            }
        }
        else
        {
            iRtn = GetLastError();
        }
    }
    else
    {
        iRtn = GetLastError();
    }
#elif defined(Q_OS_LINUX)
#else
#endif //#if defined(Q_OS_WINDOWS)

    // Update the clock time.
    g_pCVars->mRuntime = clock();

    // Done!
    return iRtn;
}


Error_t CRuntimeTracker::DumpStack()
{
    Error_t iRtn = SUCCESS;
    const size_t ciMaxFrames = 63;
    const size_t ciMaxBuffSz = (0xFF) - 1;

#if defined(Q_OS_WINDOWS)
    PVOID* pFrames = new PVOID[ciMaxFrames];

    if (nullptr != pFrames)
    {
        memset(pFrames, 0, sizeof(PVOID)*ciMaxFrames);

        size_t iCapFrames = CaptureStackBackTrace(0, ciMaxFrames, pFrames, NULL);

        SYMBOL_INFO *pSymbol = new SYMBOL_INFO;
        if (nullptr != pSymbol)
        {
            pSymbol->MaxNameLen = 255;
            pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

            char* pData = new char[ciMaxBuffSz];
            memset(pData, 0, ciMaxBuffSz);

            if (nullptr != pData)
            {
                std::string lDumpStr = "";
                for (size_t iIdx = 1; iIdx < iCapFrames; ++iIdx) // We start at 1 since we don't want to dump this function as part of the stack.
                {
                    SymFromAddr(GetCurrentProcess(), reinterpret_cast<DWORD64>(pFrames[iIdx]), 0, pSymbol);
                    sprintf(pData, "\t%zd: [0x%llX] %s\n", (iCapFrames - iIdx - 1), pSymbol->Address, pSymbol->Name);
                    lDumpStr.append(pData);
                }

                // Log the data!
#if defined(HAS_LOGGER)
#else
                lDumpStr.append("\n");
                fprintf(stdout, lDumpStr.c_str());
#endif //#if defined(HAS_LOGGER)

                if (nullptr != pData) { delete[] pData; }
            }
            else
            {
                iRtn = Err_Ptr_AllocFail;
            }

//            delete pSymbol;
        }
        else
        {
            iRtn = Err_Ptr_AllocFail;
        }

        delete[] pFrames;
    }
    else
    {
        iRtn = Err_Ptr_AllocFail;
    }
#elif defined(Q_OS_LINUX)
//        backtrace(&sStack.mpStack, ciMaxFrames);
#elif defined(Q_OS_OSX)
#endif //#if defined(Q_OS_WINDOWS).

    return iRtn;
}

Error_t CRuntimeTracker::DumpStats(bool iReset)
{
    Error_t iRtn = SUCCESS;
    const size_t ciMaxBuffSz = 65535;

    // Allocate a temporary buffer.
    char* pTmpBuff = new char[ciMaxBuffSz];
    memset(pTmpBuff, 0, ciMaxBuffSz);

    std::string lStatsLine = "Dumping Runtime Statistics...\n";

    // Time running.
    sprintf(pTmpBuff, "Runing time (CPU clocks): %ld\n", g_pCVars->mRuntime);
    lStatsLine.append(pTmpBuff);
    memset(pTmpBuff, 0, ciMaxBuffSz);

    // CPU stats.
    sprintf(pTmpBuff, "System CPU Usage: %f\n", g_pCVars->mCPUSysUsage);
    lStatsLine.append(pTmpBuff);
    memset(pTmpBuff, 0, ciMaxBuffSz);

    // Memory statistics.
    sprintf(pTmpBuff, "System Memory Total (MiB): %f\n", g_pCVars->mMemSysTotal);
    lStatsLine.append(pTmpBuff);
    memset(pTmpBuff, 0, ciMaxBuffSz);

    sprintf(pTmpBuff, "System Memory Used (MiB): %f\n", g_pCVars->mMemSysUsed);
    lStatsLine.append(pTmpBuff);
    memset(pTmpBuff, 0, ciMaxBuffSz);

    sprintf(pTmpBuff, "System Memory Free (MiB): %f\n", g_pCVars->mMemSysFree);
    lStatsLine.append(pTmpBuff);
    memset(pTmpBuff, 0, ciMaxBuffSz);

    sprintf(pTmpBuff, "Process Memory Used (MiB): %f\n", g_pCVars->mMemProcUsed);
    lStatsLine.append(pTmpBuff);
    memset(pTmpBuff, 0, ciMaxBuffSz);

    // Network statistics.
    sprintf(pTmpBuff, "Network Input (KiB): %f\n", g_pCVars->mNetIn);
    lStatsLine.append(pTmpBuff);
    memset(pTmpBuff, 0, ciMaxBuffSz);

    sprintf(pTmpBuff, "Network Output (KiB): %f\n", g_pCVars->mNetOut);
    lStatsLine.append(pTmpBuff);
    memset(pTmpBuff, 0, ciMaxBuffSz);

    // Complete! Dump the data to the log.
#if defined(HAS_LOGGER)
#else
    fprintf(stdout, lStatsLine.c_str());
#endif //#if defined(HAS_LOGGER)

    return iRtn;
}

std::string CRuntimeTracker::ErrorToString(Error_t iErr)
{
    // Clear the old error.
    mErrStr.clear();

    switch(iErr)
    {
        // Errors.
        case Err_Sig_Hangup: mErrStr = "Recieved a hangup signal! (SIGHUP)\n";
        case Err_Sig_Quit: mErrStr = "Recieved a quit signal! (SIGQUIT)\n";
        case Err_Sig_Illegal: mErrStr = "Recieved an illegal instruction signal! (SIGILL)\n";
        case Err_Sig_Abort: mErrStr = "Recieved an abort signal! (SIGABRT)\n";
        case Err_Sig_FloatExcept: mErrStr = "Recieved a floating point exception signal! (SIGFPE)\n";
        case Err_Sig_Kill: mErrStr = "Recieved a kill signal! (SIGKILL)\n";
        case Err_Sig_Bus: mErrStr = "Recieved a bus error signal! (SIGBUS)\n";
        case Err_Sig_SegFault: mErrStr = "Recieved a segmentation fault signal! (SIGSEGV)\n";
        case Err_Sig_SysCall: mErrStr = "Recieved a missing system call signal! (SIGSYS)\n";
        case Err_Sig_Pipe: mErrStr = "Recieved a broken pipe signal! (SIGPIPE)\n";
        case Err_Sig_Alarm: mErrStr = "Recieved an alarm signal! (SIGALRM)\n";
        case Err_Sig_Terminate: mErrStr = "Recieved a terminate signal! (SIGTERM)\n";
        case Err_Sig_XCPU: mErrStr = "Recieved an excessive CPU signal! (SIGXCPU)\n";
        case Err_Sig_FSizeLimit: mErrStr = "Recieved an excessive filesize signal! (SIGXFSZ)\n";
        case Err_Sig_VirtAlarm: mErrStr = "Recieved a virtual alarm signal! (SIGVTALRM)\n";
        case Err_Sig_ProfAlarm: mErrStr = "Recieved a profile alarm signal! (SIGPROF)\n";
        case Err_Ptr_AllocFail: mErrStr = "Was unable to allocate space for a pointer in the system!\n";
        case Err_Ptr_AccessVio: mErrStr = "Was unable to access an invalid address or dangling pointer!\n";
        case Err_OS_FuncFail: mErrStr = "An internal function has failed!\n";

        // Warnings.
        case Warn_LowMemory: mErrStr = "System is low on memory";
        case Warn_LowFD: mErrStr = "System is low on file descriptors";
        case Warn_LowThreads: mErrStr = "System is low on available threads";
        case Warn_LowSpace: mErrStr = "System is low on HDD space";
        case Warn_INetDown: mErrStr = "System doesn't have an network connection";
        case Warn_INetLimit: mErrStr = "System seems to have a LAN connection, but no internet.";
        default: mErrStr.clear(); // This cleared AGAIN just to be sure it's empty.
    }

    if (0 >= mErrStr.size())
    {
        // We need to query the OS for an error string!
#if defined(Q_OS_WINDOWS)
        const size_t ciMaxBuffSz = 65535;

        // Allocate a temporary buffer.
        char* pTmpBuff = new char[ciMaxBuffSz];
        memset(pTmpBuff, 0, ciMaxBuffSz);

        // Grab the message.
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, iErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), pTmpBuff, ciMaxBuffSz, NULL);

        mErrStr.append(pTmpBuff);

        if (nullptr != pTmpBuff) { delete[] pTmpBuff; }
#elif defined(Q_OS_LINUX)
#else
#endif //#if defined(Q_OS_WINDOWS)
    }

    if (0 >= mErrStr.size())
    {
        mErrStr = "An unknown error has occured!\n";
    }

    return mErrStr;
}

std::string CRuntimeTracker::GetLastErrorString()
{
    return mErrStr;
}
