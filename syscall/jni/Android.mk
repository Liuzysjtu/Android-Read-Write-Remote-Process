LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
LOCAL_MODULE := syscall
LOCAL_SRC_FILES := Syscall.c
  
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog  
  
include $(BUILD_EXECUTABLE)  