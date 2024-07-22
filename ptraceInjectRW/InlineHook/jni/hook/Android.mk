LOCAL_PATH := $(call my-dir)  


include $(CLEAR_VARS)

LOCAL_CPPFLAGS +=  -g -O0
LOCAL_ARM_MODE := arm
LOCAL_MODULE    := Inline
LOCAL_SRC_FILES := Inline.c
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

include $(BUILD_STATIC_LIBRARY)
