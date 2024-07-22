#ifndef _INLINE_H
#define _INLINE_H

#include <stdio.h>
#include <Android/log.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <stdbool.h>

#ifndef BYTE
#define BYTE unsigned char
#endif


#define LOG_TAG "CPP"
#define LOGI(format, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format, ##args)

#define PAGE_START(addr)	(~(PAGE_SIZE - 1) & (addr))
#define SET_BIT0(addr)		(addr | 1)
#define CLEAR_BIT0(addr)	(addr & 0xFFFFFFFE)
#define TEST_BIT0(addr)		(addr & 1)


/* common function */
bool ChangePageProperty(void *pAddress, size_t size);

extern void * GetModuleBaseAddr(pid_t pid, char* pszModuleName);


#endif