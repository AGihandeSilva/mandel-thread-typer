QT += widgets

HEADERS += \
    include/buttonuser.h \
    include/computeddatasegment.h \
    include/ComputeTaskGenerator.h \
    include/EditMenu.h \
    include/filemenu.h \
    include/informationdisplay.h \
    include/MandelbrotGuiTools.h \
    include/mandelbrotrenderer.h \
    include/mandelbrotwidget.h \
    include/ParameterMaker.h \
    include/ParametersMenu.h \
    include/PrecisionHandler.h \
    include/radiointegerbutton.h \
    include/regionattributes.h \
    include/RendererConfig.h \
    include/RenderHistory.h \
    include/RenderMenu.h \
    include/RenderParametersWidget.h \
    include/renderthread.h \
    include/renderthreadmediator.h \
    include/renderworker.h \
    include/settingshandler.h \
    include/settingsuser.h \
    include/threadiconmap.h \
    include/toolsmenu.h \
    include/toolsoptionswidget.h \
    include/windowmenu.h \
    include/windowthreadinfo.h \
    include/windowthreadinfokey.h \
    include/workerthreaddata.h

SOURCES       = src/main.cpp \
    src/computeddatasegment.cpp \
    src/EditMenu.cpp \
    src/filemenu.cpp \
    src/informationdisplay.cpp \
    src/MandelbrotGuiTools.cpp \
    src/mandelbrotrenderer.cpp \
    src/mandelbrotwidget.cpp \
    src/ParametersMenu.cpp \
    src/PrecisionHandler.cpp \
    src/radiointegerbutton.cpp \
    src/regionattributes.cpp \
    src/RendererConfig.cpp \
    src/RenderHistory.cpp \
    src/RenderMenu.cpp \
    src/RenderParametersWidget.cpp \
    src/renderthread.cpp \
    src/renderthreadmediator.cpp \
    src/renderworker.cpp \
    src/settingshandler.cpp \
    src/threadiconmap.cpp \
    src/toolsmenu.cpp \
    src/toolsoptionswidget.cpp \
    src/windowmenu.cpp \
    src/windowthreadinfo.cpp \
    src/windowthreadinfokey.cpp \
    src/workerthreaddata.cpp

OTHER_FILES +=  src/ComputeTaskGenerator.cpp

DEFINES += "USE_BOOST_MULTIPRECISION=1"

unix:!mac:!vxworks:!integrity:!haiku:LIBS += -lm -lquadmath

CONFIG += c++17

# install
target.path = $$[QT_INSTALL_EXAMPLES]/corelib/threads/mandelbrot
INSTALLS += target

INCLUDEPATH += "../" \
               "./src" \
               "./include"

win32:RC_ICONS = mandelbrot.ico

#QMAKE_CXXFLAGS += /ZI

DISTFILES +=

RESOURCES += \
    mandelbrotresources.qrc
