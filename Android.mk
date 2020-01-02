LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cpp .cc

LOCAL_SRC_FILES = mousegest.cpp

LOCAL_MODULE := mousegest

include $(BUILD_EXECUTABLE)
