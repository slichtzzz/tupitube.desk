# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./src/plugins/tools/viewtool
# Target is a library:  

INSTALLS += target 
target.path = /plugins/ 

macx {
    CONFIG += plugin

    INSTALLS += otool
    otool.path = /lib/
    otool.extra  += ../../../../tools/update_dylib_path.rb $(INSTALL_ROOT)/lib/$(TARGET) '^/lib' '$(INSTALL_ROOT)/lib'
}

HEADERS += viewtool.h \
           zoomconfigurator.h
SOURCES += viewtool.cpp \ 
           zoomconfigurator.cpp

CONFIG += plugin warn_on
TEMPLATE = lib
TARGET = viewtool

FRAMEWORK_DIR = "../../../framework"
include($$FRAMEWORK_DIR/framework.pri)
include(../tools_config.pri)

include(../../../../tupiglobal.pri)
