QT += widgets

HEADERS       = mandelbrotwidget.h \
                renderthread.h \
    computeddatasegment.h \
    workerthreaddata.h \
    informationdisplay.h \
    mandelbrotrenderer.h \
    toolsmenu.h \
    filemenu.h \
    regionattributes.h \
    toolsoptionswidget.h \
    radiointegerbutton.h \
    windowthreadinfo.h \
    windowmenu.h \
    renderthreadmediator.h \
    threadiconmap.h \
    windowthreadinfokey.h \
    settingshandler.h \
    settingsuser.h \
    buttonuser.h \
    renderworker.h \
    RendererConfig.h \
    RenderMenu.h \
    RenderHistory.h \
    EditMenu.h \
    ComputeTaskGenerator.h \
    ParameterMaker.h
SOURCES       = main.cpp \
                mandelbrotwidget.cpp \
                renderthread.cpp \
    computeddatasegment.cpp \
    workerthreaddata.cpp \
    informationdisplay.cpp \
    mandelbrotrenderer.cpp \
    toolsmenu.cpp \
    filemenu.cpp \
    regionattributes.cpp \
    toolsoptionswidget.cpp \
    radiointegerbutton.cpp \
    windowthreadinfo.cpp \
    windowmenu.cpp \
    renderthreadmediator.cpp \
    threadiconmap.cpp \
    windowthreadinfokey.cpp \
    settingshandler.cpp \
    renderworker.cpp \
    RendererConfig.cpp \
    RenderMenu.cpp \
    RenderHistory.cpp \
    EditMenu.cpp

OTHER_FILES +=  ComputeTaskGenerator.cpp

DEFINES += "USE_BOOST_MULTIPRECISION=1"

unix:!mac:!vxworks:!integrity:!haiku:LIBS += -lm

CONFIG += c++17

# install
target.path = $$[QT_INSTALL_EXAMPLES]/corelib/threads/mandelbrot
INSTALLS += target

INCLUDEPATH += "../"

win32:RC_ICONS = mandelbrot.ico

#QMAKE_CXXFLAGS += /ZI

DISTFILES +=

RESOURCES += \
    mandelbrotresources.qrc
