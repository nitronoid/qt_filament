# We want to build an executable
TEMPLATE = app
# Specify where we should store our build files
BUILD_PATH = build
# Name our executable
TARGET = $${BUILD_PATH}/bin/QtFilament


# Specify where to find the ui forms
UI_HEADERS_DIR = ui
UI_DIR = ui
# Specify where to write our object files
OBJECTS_DIR = $${BUILD_PATH}/obj
MOC_DIR = $${BUILD_PATH}/moc

QT += opengl core gui
CONFIG += console c++14
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
    $$PWD/include \
    $$PWD/ui \
    $$PWD/materials \
    ${FILAMENT_PATH}/include 

HEADERS += $$files(include/*.h, true)
SOURCES += $$files(src/*.cpp, true)

FORMS += ui/applayout.ui

LIBS += -L${FILAMENT_PATH}/lib/x86_64
LIBS += \
  -lfilament \
  -lbluegl \
  -lbluevk \
  -lfilabridge \
  -lfilaflat \
  -lutils \
  -lgeometry \
  -lsmol-v \
  -ldl \
  -pthread 

if (!isEmpty(STDLIB_PATH)) {
    LIBS += -L${STDLIB_PATH}
}
# Need this to find filament symbols
LIBS += -lc++ 

macx:{
    QMAKE_CXXFLAGS += -x objective-c++
    QMAKE_LFLAGS += -framework Metal -framework MetalKit -framework Cocoa -framework CoreFoundation -fobjc-link-runtime
}

# use clang, dont need to 
QMAKE_CXXFLAGS += -Ofast -msse -msse2 -msse3 -march=native -funroll-loops 
QMAKE_CXXFLAGS += -Wall -Wextra

