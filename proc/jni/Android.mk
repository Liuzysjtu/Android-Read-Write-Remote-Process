LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
LOCAL_MODULE := proc
LOCAL_SRC_FILES := Proc.c
  
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog  
  
include $(BUILD_EXECUTABLE)  