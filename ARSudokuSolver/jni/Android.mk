
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include /home/john/OpenCV/android/build/android-opencv.mk
LOCAL_LDLIBS += $(OPENCV_LIBS)
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_C_INCLUDES += $(OPENCV_LIBS)
LOCAL_MODULE    := sudokudetector
LOCAL_SRC_FILES := SudokuDetector.cpp


#LOCAL_STATIC_LIBRARIES := opencv_core opencv_ml opencv_features2d

include $(BUILD_SHARED_LIBRARY)