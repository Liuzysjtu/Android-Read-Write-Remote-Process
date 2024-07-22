LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
LOCAL_MODULE := ptrace
LOCAL_SRC_FILES := Ptrace.c PtraceMain.c
  
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
  
include $(BUILD_EXECUTABLE)  