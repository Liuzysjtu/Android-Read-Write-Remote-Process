/************************************************************
  FileName: InjectModule.c
  Description:       ptrace注入      
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <PrintLog.h>

#define  MAX_PATH 0x100

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

    long DestBuf;
    long WriteData = 0x005B2301;

    char mem_path[256];
    sprintf(mem_path, "/proc/%d/mem", pid);

    int fd = open(mem_path, O_RDWR);

    // read mem
    lseek64(fd, readAddr, SEEK_SET);
    read(fd, &DestBuf, 4);

    printf("DestBuf is 0x%08X.\n", DestBuf);

    sleep(1);

    // write mem
    lseek64(fd, readAddr, SEEK_SET);
    write(fd, &WriteData, 4);

    sleep(1);

    // read mem again
    lseek64(fd, readAddr, SEEK_SET);
    read(fd, &DestBuf, 4);

    printf("DestBuf after hooking is 0x%08X.\n", DestBuf);

    close(fd);

    return 0;
}  