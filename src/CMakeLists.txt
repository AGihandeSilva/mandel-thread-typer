cmake_minimum_required(VERSION 3.1.0)

project(mandelbrot)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
#handle resources automatically
set(CMAKE_AUTORCC ON)

if (MSVC)
set(CMAKE_VS_PLATFORM_NAME x64)
endif (MSVC)

if (UNIX)
set(CMAKE_CXX_STANDARD 14)
endif (UNIX)

#add header files to IDE project (no target for them)
FILE(GLOB_RECURSE myHeaderFiles "*.h")
add_custom_target(headers SOURCES ${myHeaderFiles})

#templated function code
add_custom_target(AdditionalCode SOURCES ComputeTaskGenerator.cpp)

# Find the QtWidgets library
#find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# Populate a CMake variable with the sources
set(mandelbrot_SRCS
    computeddatasegment.cpp
    EditMenu.cpp
    filemenu.cpp
    informationdisplay.cpp
    main.cpp
    mandelbrotrenderer.cpp
    mandelbrotwidget.cpp
    radiointegerbutton.cpp
    regionattributes.cpp
    RendererConfig.cpp
    RenderHistory.cpp
    RenderMenu.cpp
    renderthread.cpp
    renderthreadmediator.cpp
    renderworker.cpp
    settingshandler.cpp
    threadiconmap.cpp
    toolsmenu.cpp
    toolsoptionswidget.cpp
    windowmenu.cpp
    windowthreadinfo.cpp
    windowthreadinfokey.cpp
    workerthreaddata.cpp
	mandelbrotresources.qrc
	mandelbrot.rc
)
# Tell CMake to create the mandelbrot executable
add_executable(mandelbrot WIN32 ${mandelbrot_SRCS} mandelbrot.rc)
# Use the Widgets module from Qt 5
target_link_libraries(mandelbrot Qt5::Widgets)

if (UNIX)
target_link_libraries(mandelbrot -lpthread)
endif (UNIX)

#add boost for extended type support (as desired)
#add the required boost header files with the 
#parent 'boost' directory at the same level as this project directory
#and edit the line below to enable/disable the use of these types
set (USE_EXTENDED_TYPES 1)

if (${USE_EXTENDED_TYPES})
include_directories(../)
add_definitions(-DUSE_BOOST_MULTIPRECISION=1)
endif()
