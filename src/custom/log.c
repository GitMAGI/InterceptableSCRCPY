#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#if defined _WIN64 || defined _WIN32
    #include <strings.h>
    #include <windows.h>
#else
	#include <string.h>
#endif

#include "log.h"
#include "util.h"
#include "config.h"

void debugLog_(char *msg, const char* caller){
	writeLog(msg, caller, LDEBUG);
}

void infoLog_(char *msg, const char* caller){
	writeLog(msg, caller, LINFO);
}

void warningLog_(char *msg, const char* caller){
	writeLog(msg, caller, LWARNING);
}

void errorLog_(char *msg, const char* caller){
	writeLog(ssprintf("%s. Error Code: %d. Error Description: %s", msg, errno, strerror(errno)), caller, LERROR);
}

void criticalLog_(char *msg, const char* caller){
	writeLog(ssprintf("%s. Error Code: %d. Error Description: %s", msg, errno, strerror(errno)), caller, LCRITICAL);
}

void writeLog(char* msg, const char* scope, LogType type){
	bool file = false;
	if(strlen(LOGFILEPATH) > 0)
		file = true;

	char* str = "DEBUG";
	switch(type)
    {
		case LDEBUG: 
			str = "DEBUG";        
			break;
		case LINFO: 
			str = "INFO";
			break;
		case LWARNING: 
			str = "WARNING";
			break;
		case LERROR: 
			str = "ERROR";
			break;
		case LCRITICAL: 
			str = "CRITICAL";
			break;
        default:
            str = "DEBUG";
    }

    char* toLog = toLog = ssprintf("%s | %s | %s | %s() >>> %s\n", getCurrentTimeStamp(), str, APPNAME, scope, msg);
    fprintf(stdout, toLog);

    if(file == true){
        char* fileextension = "log";
        char* filepath = LOGFILEPATH;
        char* filejustname = getCurrentTimeStampForFileName();

        char* filename = ssprintf("%s%s.%s", filepath, filejustname, fileextension);
		if(!directoryExists(filepath))
			createPath(filepath);

        FILE *fp = fopen(filename, "ab+");
        if(fp){            
            fprintf(fp, toLog);
        }
        else{
			fprintf(stderr, "Logging Error on file %s", filename);
        }
        
    }
}