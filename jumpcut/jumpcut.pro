QT -= core
QT -= gui

CONFIG += c++11

TARGET = jumpcut
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    BITMAP3.cpp \
    cuda.cpp \
    delaunay.cpp \
    imgwarp_mls.cpp \
    imgwarp_mls_rigid.cpp \
    imgwarp_mls_similarity.cpp \
    imgwarp_piecewiseaffine.cpp \
    videoCutout.cpp

HEADERS += \
    BITMAP3.h \
    cuda.h \
    delaunay.h \
    fpm.h \
    imgwarp_mls.h \
    imgwarp_mls_rigid.h \
    imgwarp_mls_similarity.h \
    imgwarp_piecewiseaffine.h \
    videoCutout.h

QMAKE_CXXFLAGS += -std=c++11

OTHER_FILES += cuda.cu
#CUDA_SOURCES += cuda.cu

# DO NOT EDIT BEYOND THIS UNLESS YOU KNOW WHAT YOU ARE DOING....
win32 {
    INCLUDEPATH += "C:/ProgramData/NVIDIA Corporation/CUDA Samples/v7.5/common/inc"
    CUDA_SDK = 'C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v7.5/'   # Path to cuda SDK install
    CUDA_DIR = 'C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v7.5/'            # Path to cuda toolkit install
    SYSTEM_NAME = Win64         # Depending on your system either 'Win32', 'x64', or 'Win64'
    SYSTEM_TYPE = 64            # '32' or '64', depending on your system
    CUDA_ARCH = sm_21           # Type of CUDA architecture, for example 'compute_10', 'compute_11', 'sm_10'
    NVCC_OPTIONS = --use_fast_math
    NVCC_OPTIONS += -ccbin "\"D:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/x86_amd64/cl.exe\"" -Xcompiler "\"/EHsc /W3 /nologo /O2 /Zi  /MD \""
} else {
    INCLUDEPATH += "/usr/local/cuda-8.0/samples/common/inc/"

    CUDA_SDK = '/usr/local/cuda-8.0/'   # Path to cuda SDK install
    CUDA_DIR = '/usr/local/cuda-8.0/'            # Path to cuda toolkit install
    SYSTEM_NAME = Linux         # Depending on your system either 'Win32', 'x64', or 'Win64'
    SYSTEM_TYPE = 64            # '32' or '64', depending on your system
    CUDA_ARCH = sm_21           # Type of CUDA architecture, for example 'compute_10', 'compute_11', 'sm_10'
    NVCC_OPTIONS = --use_fast_math
    NVCC_OPTIONS += -Xcompiler "\"-O2\""
}
CUDA_DIR = '/usr/local/cuda-8.0/'            # Path to cuda toolkit install

# include paths
INCLUDEPATH += $$CUDA_DIR/include

# library directories
QMAKE_LIBDIR += $$CUDA_DIR/lib/x64

CUDA_OBJECTS_DIR = ./


# Add the necessary libraries
CUDA_LIBS = -lcuda -lcudart

# The following makes sure all path names (which often include spaces) are put between quotation marks
CUDA_INC = $$join(INCLUDEPATH,'" -I"','-I"','"')
#LIBS += $$join(CUDA_LIBS,'.so ', '', '.so')
LIBS += $$CUDA_LIBS


# Configuration of the Cuda compiler
CONFIG(debug, debug|release) {
    # Debug mode
    cuda_d.input = OTHER_FILES
    cuda_d.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
    cuda_d.commands = $$CUDA_DIR/bin/nvcc -D_DEBUG $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda_d.dependency_type = TYPE_C
    QMAKE_EXTRA_COMPILERS += cuda_d
}
else {
    # Release mode
    cuda.input = OTHER_FILES
    cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
    cuda.commands = $$CUDA_DIR/bin/nvcc $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda.dependency_type = TYPE_C
    QMAKE_EXTRA_COMPILERS += cuda
}

win32 {
        LIBS += -LD:\OpenCV2.4\opencv\build\x64\vc12\lib -lopencv_core2410 -lopencv_imgproc2410 -lopencv_highgui2410 -lopencv_calib3d2410 \
-lopencv_contrib2410 -lopencv_core2410 -lopencv_features2d2410 -lopencv_flann2410 -lopencv_gpu2410 -lopencv_highgui2410 -lopencv_imgproc2410 -lopencv_legacy2410 -lopencv_ml2410 -lopencv_nonfree2410 -lopencv_objdetect2410 -lopencv_ocl2410 -lopencv_photo2410 -lopencv_stitching2410 -lopencv_superres2410 -lopencv_ts2410 -lopencv_video2410 -lopencv_videostab2410
        INCLUDEPATH += D:\OpenCV2.4\opencv\build\include\opencv
        INCLUDEPATH += D:\OpenCV2.4\opencv\build\include
        LIBS += -L..\build-FPMLib-Desktop_Qt_5_6_2_MSVC2013_64bit-Release\release -lFPMLib
} else {
        INCLUDEPATH += /usr/local/include/opencv/
        INCLUDEPATH += /usr/local/include/
        #QMAKE_CXXFLAGS += -fpermissive
        #QMAKE_CXX = g++-5
        LIBS += -L/usr/local/lib/ -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lopencv_imgcodecs -lopencv_xfeatures2d -lopencv_ximgproc
        LIBS += -L../FPMLib -lFPMLib -L/usr/lib/x86_64-linux-gnu/ -lfftw3f -lfftw3
}
