/*******************************
 *  FileName: Ptrace.c
 *  Description:       ptrace读写
 * *****************************/


#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>
#include <PrintLog.h>
#include <Ptrace.h>

/***************************
 *  Description:    使用ptrace Attach附加到指定进程,发送SIGSTOP信号给指定进程让其停止下来并对其进行跟踪。
 *                  但是被跟踪进程(tracee)不一定会停下来，因为同时attach和传递SIGSTOP可能会将SIGSTOP丢失。
 *                  所以需要waitpid(2)等待被跟踪进程被停下
 *  Input:          pid表示远程进程的ID
 *  Output:         无
 *  Return:         返回0表示attach成功，返回-1表示失败
 *  Others:         无
 * ************************/
int ptrace_attach(pid_t pid) {
    int status = 0;
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        LOGD("ptrace attach error, pid:%d", pid);
        return -1;
    }

    LOGD("attach process pid:%d", pid);
    waitpid(pid, &status, WUNTRACED);
    return 0;
}

/*************************************************
 *   Description:    使用ptrace detach指定进程,完成对指定进程的跟踪操作后，使用该参数即可解除附加
 *   Input:          pid表示远程进程的ID
 *   Output:         无
 *   Return:         返回0表示detach成功，返回-1表示失败
 *   Others:         无
 * ***********************************************/
int ptrace_detach(pid_t pid) {
    if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0) {
        LOGD("detach process error, pid:%d", pid);
        return -1;
    }
    LOGD("detach process pid:%d", pid);
    return 0;
}

/*************************************************
 *   Description:    使用ptrace从远程进程内存中读取数据
 *   Input:          pid表示远程进程的ID，pSrcBuf表示从远程进程读取数据的内存地址
 *                   pDestBuf表示用于存储读取出数据的地址，size表示读取数据的大小
 *   Return:         返回0表示读取数据成功
 *   other:          这里的*_t类型是typedef定义一些基本类型的别名，用于跨平台。例如
 *                   uint8_t表示无符号8位也就是无符号的char类型
 * **********************************************/
int ptrace_readdata(pid_t pid, uint8_t *pSrcBuf, uint8_t *pDestBuf, uint32_t size) {
    uint32_t nReadCount = 0;
    uint32_t nRemainCount = 0;
    uint8_t *pCurSrcBuf = pSrcBuf;
    uint8_t *pCurDestBuf = pDestBuf;
    long lTmpBuf = 0;
    uint32_t i = 0;

    //每次读取4字节数据
    nReadCount = size / sizeof(long);
    nRemainCount = size % sizeof(long);
    for (i = 0; i < nReadCount; i++) {
        lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurSrcBuf, 0);
        memcpy(pCurDestBuf, (char *) (&lTmpBuf), sizeof(long));
        pCurSrcBuf += sizeof(long);
        pCurDestBuf += sizeof(long);
    }
    //当最后读取的字节不足4字节时调用
    if (nRemainCount > 0) {
        lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurSrcBuf, 0);
        memcpy(pCurDestBuf, (char *) (&lTmpBuf), nRemainCount);
    }
    return 0;
}

/*************************************************
 *  Description:    使用ptrace将数据写入到远程进程空间中
 *  Input:          pid表示远程进程的ID，pWriteAddr表示写入数据到远程进程的内存地址
 *                  pWriteData用于存储写入数据的地址，size表示写入数据的大小
 *  Return:         返回0表示写入数据成功，返回-1表示写入数据失败 
 * ***********************************************/
int ptrace_writedata(pid_t pid, uint8_t *pWriteAddr, uint8_t *pWriteData, uint32_t size) {
    uint32_t nWriteCount = 0;
    uint32_t nRemainCount = 0;
    uint8_t *pCurSrcBuf = pWriteData;
    uint8_t *pCurDestBuf = pWriteAddr;
    long lTmpBuf = 0;
    uint32_t i = 0;

    nWriteCount = size / sizeof(long);
    nRemainCount = size % sizeof(long);

    //数据以sizeof(long)字节大小为单位写入到远程进程内存空间中
    for (i = 0; i < nWriteCount; i++) {
        memcpy((void *) (&lTmpBuf), pCurSrcBuf, sizeof(long));
        if (ptrace(PTRACE_POKETEXT, pid, pCurDestBuf, lTmpBuf) < 0) {
            LOGD("Write Remote Memory error, MemoryAddr:0x%lx", (long) pCurDestBuf);
            return -1;
        }
        pCurSrcBuf += sizeof(long);
        pCurDestBuf += sizeof(long);
    }
    if (nRemainCount > 0) {
        //lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurDestBuf, NULL);
        memcpy((void *) (&lTmpBuf), pCurSrcBuf, nRemainCount);
        if (ptrace(PTRACE_POKETEXT, pid, pCurDestBuf, lTmpBuf) < 0) {
            LOGD("Write Remote Memory error, MemoryAddr:0x%lx", (long) pCurDestBuf);
            return -1;
        }
    }
    return 0;
}

/*************************************************
 *  Description:    在指定进程中搜索对应模块的基址
 *  Input:          pid表示远程进程的ID，若为-1表示自身进程，ModuleName表示要搜索的模块的名称
 *  Return:         返回0表示获取模块基址失败，返回非0为要搜索的模块基址
 * **********************************************/
void *GetModuleBaseAddr(pid_t pid, const char *ModuleName) {
    char szFileName[50] = {0};
    FILE *fp = NULL;
    char szMapFileLine[1024] = {0};
    char *ModulePath, *MapFileLineItem;
    long ModuleBaseAddr = 0;

    // 读取"/proc/pid/maps"可以获得该进程加载的模块
    if (pid < 0) {
        snprintf(szFileName, sizeof(szFileName), "/proc/self/maps");
    } else {
        snprintf(szFileName, sizeof(szFileName), "/proc/%d/maps", pid);
    }

    fp = fopen(szFileName, "r");
    if (fp != NULL) {
        while (fgets(szMapFileLine, sizeof(szMapFileLine), fp)) {
            if (strstr(szMapFileLine, ModuleName)) {
                MapFileLineItem = strtok(szMapFileLine, " \t");
                char *Addr = strtok(szMapFileLine, "-");
                ModuleBaseAddr = strtoul(Addr, NULL, 16);

                if (ModuleBaseAddr == 0x8000) {
                    ModuleBaseAddr = 0;
                }
                break;
            }
        }
        fclose(fp);
    }
    return (void *) ModuleBaseAddr;
}