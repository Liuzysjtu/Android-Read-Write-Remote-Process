/**********************************
 *  FileName:   Ptrace.h
 *  Decription: ptrace读写
 * ********************************/

#include <stdio.h>    
#include <stdlib.h>       
#include <unistd.h> 

#define  MAX_PATH 0x100

void *GetModuleBaseAddr(pid_t pid, const char *ModuleName);

int ptrace_writedata(pid_t pid, uint8_t *pWriteAddr, uint8_t *pWriteData, uint32_t size);

int ptrace_readdata(pid_t pid, uint8_t *pSrcBuf, uint8_t *pDestBuf, uint32_t size);

int ptrace_detach(pid_t pid);

int ptrace_attach(pid_t pid);