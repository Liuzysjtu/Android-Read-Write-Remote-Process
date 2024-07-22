#include <vector>

extern "C"
{
#include "Inline.h"
}

//声明函数在加载库时被调用,也是hook的主函数
void Modify2048() __attribute__((constructor));


/**
 *  1.Hook入口
 */
void Modify2048() {
    uint32_t offset = 0x000a1a46;

    LOGI("In IHook's Modify2048.");
    void *pModuleBaseAddr = GetModuleBaseAddr(-1, "libcocos2dcpp.so");
    LOGI("libcocos2dcpp.so base addr is 0x%X.", pModuleBaseAddr);
    if (pModuleBaseAddr == 0) {
        LOGI("get module base error.");
        return;
    }

    //模块基址加上HOOK点的偏移地址就是HOOK点在内存中的位置
    uint32_t uiHookAddr = (uint32_t) pModuleBaseAddr + offset;

    long DestBuf;

    SafeMemRead(uiHookAddr, 4, &DestBuf);
    LOGI("DestBuf is 0x%08X.", DestBuf);


    DestBuf = 0x005B2301;
    SafeMemWrite(uiHookAddr, 4, &DestBuf);

    SafeMemRead(uiHookAddr, 4, &DestBuf);
    LOGI("DestBuf is 0x%08X.", DestBuf);
}

