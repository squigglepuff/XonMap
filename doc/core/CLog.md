**class CLog**
================

*description*
----------------
This class is responsible for handling all logging done by the application/system. This class doesn't handle errors and is "dumb", meaning it's ignorant of the rest of the runtime. It will only do as simply told and nothing else. The class is responsible for logging to stdout/stderr (if available) and appending to a file. The class also performs log rotation after a certain size is reached, defined in a configuration file (~/.xonmapcfg), or once the file reaches 100MB by default. The log rotation GZips the files (as GZip uses DEFLATE, making txt files quite small) and appends an EPOCH rotation timestamp to the filename. All these log files will be written to the following path(s):
*Windows:* %PROGRAMDATA%\xonmap\logs
*Linux:* ~/.xonmap/logs
*MacOS:* ~/Library/xonmap/logs

*API*
================
The class should expose the following API to the world:
```cpp
	Error_t Init(void)
		// This function should detect the OS and run the corresponding private function to setup and initialize the logging system.

	Error_t Reload(void)
		This function resets everything back to a sane state (i.e. 0) and re-runs Init().

	Error_t Log(Level iLevel, std::string iMsg, int iLine, std::string iFile, std::string iFunc = "")
		This function actually logs to Log4Cplus using the provided data.

	The class has the following as PRIVATE:
		Error_t InitWin_p(void)
		Error_t InitLinux_p(void)
		Error_t InitApple_p(void)
			These setup the logging sub-system and get it ready to go. This includes installing the Log4Cplus configuration file, printing a logging header and installing the Qt message handler.
```

*Defines*
----------------
| Name                     | Value                                                     |
|--------------------------|-----------------------------------------------------------|
| cTrace(std::string iMsg) | Log(Level::Trace, iMsg, __LINE__, __FILE__, __FUNCTION__) |
| cDebug(std::string iMsg) | Log(Level:Debug, iMsg, __LINE__, __FILE__, __FUNCTION__)  |
| cInfo(std::string iMsg)  | Log(Level:Info, iMsg, __LINE__, __FILE__)                 |
| cWarn(std::string iMsg)  | Log(Level:Warn, iMsg, __LINE__, __FILE__)                 |
| cError(std::string iMsg) | Log(Level:Error, iMsg, __LINE__, __FILE__)                |
| cFatal(std::string iMsg) | Log(Level:Fatal, iMsg, __LINE__, __FILE__)                |

*Enumerations*
----------------

**Level**

| Enum  | Code         | Meaning                                       |
|-------|--------------|-----------------------------------------------|
| Trace | Compiler-set | TRACE a code-path                             |
| Debug | Compiler-set | Show debugging information                    |
| Info  | Compiler-set | Show some information                         |
| Warn  | Compiler-set | Indicate something isn't quite right          |
| Error | Compiler-set | Something has gone wrong                      |
| Fatal | Compiler-set | Something has gone so wrong, we need to quit! |

*Data Structures*
----------------
[NONE]
