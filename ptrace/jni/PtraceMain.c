/************************************************************
  FileName: PtraceMain.c
  Description:       ptrace读写
***********************************************************/

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
#include <Ptrace.h>
#include <PrintLog.h>

/*************************************************
  Description:    通过进程名称定位到进程的PID
  Input:          process_name为要定位的进程名称
  Output:         无
  Return:         返回定位到的进程PID，若为-1，表示定位失败
  Others:         无
*************************************************/
pid_t FindPidByProcessName(const char *process_name) {
    int ProcessDirID = 0;
    pid_t pid = -1;
    FILE *fp = NULL;
    char filename[MAX_PATH] = {0};
    char cmdline[MAX_PATH] = {0};

    struct dirent *entry = NULL;

    if (process_name == NULL)
        return -1;

    DIR *dir = opendir("/proc");
    if (dir == NULL)
        return -1;

    while ((entry = readdir(dir)) != NULL) {
        ProcessDirID = atoi(entry->d_name);
        if (ProcessDirID != 0) {
            snprintf(filename, MAX_PATH, "/proc/%d/cmdline", ProcessDirID);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);

                if (strncmp(process_name, cmdline, strlen(process_name)) == 0) {
                    pid = ProcessDirID;
                    break;
                }
            }
        }
    }

    closedir(dir);
    return pid;
}

int main(int argc, char *argv[]) {
    char InjectProcessName[MAX_PATH] = "com.estoty.game2048";   // 读取内存进程名称

    // 当前设备环境判断
#if defined(__i386__)
    LOGD("Current Environment x86");
    return -1;
#elif defined(__arm__)
    LOGD("Current Environment ARM");
#else
    LOGD("other Environment");
    return -1;
#endif

    pid_t pid = FindPidByProcessName(InjectProcessName);
    if (pid == -1) {
        printf("Get Pid Failed");
        return -1;
    }

    void *pModuleBaseAddr = GetModuleBaseAddr(pid, "libcocos2dcpp.so"); // 获取远程进程加载的模块的基址
    LOGD("libcocos2dcpp.so base addr is 0x%08X.", pModuleBaseAddr);

    uint32_t offset = 0x000a1a46;

    //模块基址加上HOOK点的偏移地址就是HOOK点在内存中的位置
    uint32_t readAddr = (uint32_t) pModuleBaseAddr + offset;

    printf("readAddr is 0x%08X.\n", readAddr);

    uint8_t *pSrcBuf = (uint8_t *) readAddr;
    long DestBuf;
    long WriteData = 0x005B2301;

    // ptrace attach
    if (ptrace_attach(pid) == -1) {
        return -1;
    }

    // ptrace read
    ptrace_readdata(pid, pSrcBuf, &DestBuf, 4);
    printf("DestBuf is 0x%08X.\n", DestBuf);

    sleep(1);

    // ptrace write
    ptrace_writedata(pid, pSrcBuf, &WriteData, 4);
    sleep(1);

    // ptrace read again
    ptrace_readdata(pid, pSrcBuf, &DestBuf, 4);
    printf("DestBuf after hooking is 0x%08X.\n", DestBuf);

    // ptrace detach
    if (ptrace_detach(pid) == -1) {
        LOGD("ptrace detach failed");
        return -1;
    }

    return 0;
}  