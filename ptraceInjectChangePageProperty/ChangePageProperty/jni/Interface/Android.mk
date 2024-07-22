LOCAL_PATH := $(call my-dir)  


include $(CLEAR_VARS)

LOCAL_CXXFLAGS +=  -g -O0
LOCAL_ARM_MODE := arm
LOCAL_MODULE    := InlineChangePageProperty
LOCAL_STATIC_LIBRARIES:= ChangePageProperty
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../hook
LOCAL_SRC_FILES := ChangePageProperty.cpp
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)