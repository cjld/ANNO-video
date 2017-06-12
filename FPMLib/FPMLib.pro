QT -= core
QT -= gui

TARGET = FPMLib
CONFIG += console
CONFIG -= app_bundle

QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = lib

SOURCES += \
    FPMLib/fft_fpm.cpp \
    FPMLib/fpm.cpp \
    FPMLib/fpmi.h \
    FPMLib/naive_fpm.cpp \
    lib/src/image.cpp \
    lib/src/iff.cpp \
    lib/src/util.cpp \
    lib/src/resample.cpp \
    lib/src/mem.cpp \
    lib/src/err.cpp

HEADERS += \
    FPMLib/fpm.h \
    lib/bfc/argv.h \
    lib/bfc/array.h \
    lib/bfc/autores.h \
    lib/bfc/bfstream.h \
    lib/bfc/cfg.h \
    lib/bfc/ctc.h \
    lib/bfc/def.h \
    lib/bfc/diskmap.h \
    lib/bfc/err.h \
    lib/bfc/filex.h \
    lib/bfc/fio.h \
    lib/bfc/fvtx.h \
    lib/bfc/fwd.h \
    lib/bfc/matrix.h \
    lib/bfc/matrixop.h \
    lib/bfc/mem.h \
    lib/bfc/stdf.h \
    lib/bfc/type.h \
    lib/bfc/util.h \
    lib/bfc/vector.h \
    lib/bfc/winx.h \
    lib/IFF/def.h \
    lib/IFF/iff.h \
    lib/IFF/image.h \
    lib/IFF/ioimpl.h \
    lib/IFF/itf.h \
    lib/IFF/opencv.h \
    lib/IFF/resample.h \
    lib/IFF/util.h \
    lib/IPF/def.h \
    lib/IPF/ipf.h \
    lib/IPF/ipt.h \
    lib/IPF/ipt_func.h \
    lib/ffdef.h

INCLUDEPATH += ./lib ../fftw-3.3.5-dll64
win32:LIBS += -L../fftw-3.3.5-dll64 -llibfftw3f-3 -llibfftw3-3
linux:LIBS += -L/usr/lib/x86_64-linux-gnu/ -lfftw3f -lfftw3
DEFINES += _IFF_STATIC _FFS_STATIC FPMLIB_EXPORT
