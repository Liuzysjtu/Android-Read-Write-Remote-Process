#include "Inline.h"

#define ALIGN_PC(pc) (pc & 0xFFFFFFFC)

/**
 *  通用函数：获取so模块加载进内存的基地址，通过查看/proc/$pid/maps文件
 *  
 *  @param  pid             模块所在进程pid，如果访问自身进程，可填小余0的值，如-1
 *  @param  pszModuleName   模块名字
 *  @return void*           模块的基地址，错误返回0 
 */
void *GetModuleBaseAddr(pid_t pid, char *pszModuleName) {
    FILE *pFileMaps = NULL;
    unsigned long ulBaseValue = 0;
    char szMapFilePath[256] = {0};
    char szFileLineBuffer[1024] = {0};

    /* 判断是否为自身maps文件*/
    if (pid < 0) {
        snprintf(szMapFilePath, sizeof(szMapFilePath), "/proc/self/maps");
    } else {
        snprintf(szMapFilePath, sizeof(szMapFilePath), "/proc/%d/maps", pid);
    }

    pFileMaps = fopen(szMapFilePath, "r");
    if (NULL == pFileMaps) {
        return (void *) ulBaseValue;
    }
    /* 循环遍历maps文件，找到对应模块名，截取字符串中的基地址*/
    while (fgets(szFileLineBuffer, sizeof(szFileLineBuffer), pFileMaps) != NULL) {
        if (strstr(szFileLineBuffer, pszModuleName)) {
            char *pszModuleAddress = strtok(szFileLineBuffer, "-");
            ulBaseValue = strtoul(pszModuleAddress, NULL, 16);

            if (ulBaseValue == 0x8000) {
                ulBaseValue = 0;
            }
            break;
        }
    }
    fclose(pFileMaps);

    return (void *) ulBaseValue;
}

/**
 * 通用函数，修改页属性，让内存块内的代码可执行
 *
 * @param   pAddress    需要修改属性起始地址
 * @param   size        需要修改页属性的长度
 * @return  bool        是否修改成功
 */
bool ChangePageProperty(void *pAddress, size_t size) {
    bool bRet = false;

    while (1) {
        if (pAddress == NULL) {
            LOGI("change page property error.");
            break;
        }

        unsigned long ulPageSize = sysconf(_SC_PAGESIZE);
        int iProtect = PROT_READ | PROT_WRITE | PROT_EXEC;
        /*页对齐，以4096的倍数为起始位置*/
        unsigned long ulNewPageStartAddress = (unsigned long) (pAddress) & ~(ulPageSize - 1);
        /* 计算至少需要多少内存页(0x1000byte)可以包含size大小*/
        long lPageCount = (size / ulPageSize) + 1;
        int iRet = mprotect((const void *) (ulNewPageStartAddress), lPageCount * ulPageSize,
                            iProtect);

        if (iRet == -1) {
            LOGI("mprotect error:%s", strerror(errno));
            break;
        }

        bRet = true;
        break;
    }

    return bRet;
}

/**
 * 通用函数，向指定内存区域写入数据
 *
 * @param   ulAddress   写入地址
 * @param   size        写入长度
 * @param   pszBuffer   写入数据
 * @return  bool        是否写入成功
 */
bool SafeMemWrite(unsigned long ulAddress, size_t size, void *pszBuffer) {
    bool bRet = false;
    do {
        if (pszBuffer == NULL) {
            LOGI("pszBuffer is null.");
            break;
        }

        /* 修改页属性，让内存块内的代码可执行*/
        if (!ChangePageProperty((void *)ulAddress, size)) {
            LOGI("ChangePageProperty error.");
            break;
        }

        memcpy((void*)ulAddress, pszBuffer, size);
        LOGI("SafeMemWrite success.");
        bRet = true;
    }while (0);
    return bRet;
}

/**
 * 通用函数，读取指定内存区域数据
 *
 * @param   ulAddress   读取地址
 * @param   size        读取长度
 * @param   pszBuffer   读取数据
 * @return  bool        是否读取成功
 */
bool SafeMemRead(unsigned long ulAddress, size_t size, void *pszBuffer) {

    memcpy(pszBuffer, (void*)ulAddress, size);
    LOGI("SafeRead success.");

    return true;
}

