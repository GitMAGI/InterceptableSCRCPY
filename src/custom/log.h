#ifndef LOG_CUSTOM_H
#define LOG_CUSTOM_H

#include <stdbool.h>
#include <stdarg.h>
#include <config.h>

typedef enum 
{
	LDEBUG = 0, 
	LINFO = 1,  
	LWARNING = 2,
	LERROR = 3,
	LCRITICAL = 4
} LogType;

/*
void debugLog(char *msg);
void infoLog(char *msg);
void warningLog(char *msg);
void errorLog(char *msg);
void criticalLog(char *msg);
*/

void debugLog_(char *msg, char const * caller_name);
void infoLog_(char *msg, char const * caller_name);
void warningLog_(char *msg, char const * caller_name);
void errorLog_(char *msg, char const * caller_name);
void criticalLog_(char *msg, char const * caller_name);

void writeLog(char* msg, const char* scope, LogType type);

#define debugLog(msg) debugLog_(msg, __FUNCTION__)
#define infoLog(msg) infoLog_(msg, __FUNCTION__)
#define warningLog(msg) warningLog_(msg, __FUNCTION__)
#define errorLog(msg) errorLog_(msg, __FUNCTION__)
#define criticalLog(msg) criticalLog_(msg, __FUNCTION__)

#endif