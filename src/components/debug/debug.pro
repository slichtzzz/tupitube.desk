# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/components/debug
# Target is a library:  

INSTALLS += target
target.path = /lib/

macx {
    CONFIG += plugin

    INSTALLS += otool
    otool.path = /lib/
    otool.extra  += ../../../tools/update_dylib_path.rb $(INSTALL_ROOT)/lib/$(TARGET) '^/lib' '$(INSTALL_ROOT)/lib'
}

HEADERS += tupdebugwidget.h \
           tupdebugterm.h

SOURCES += tupdebugwidget.cpp \
           tupdebugterm.cpp

CONFIG += dll warn_on
TEMPLATE = lib
TARGET = debug

FRAMEWORK_DIR = "../../framework"
include($$FRAMEWORK_DIR/framework.pri)
include(../components_config.pri)

