cmake_minimum_required(VERSION 3.1.0)

project(mandelbrot)

#add include directory
include_directories(./include)
include_directories(./src)

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

set(CMAKE_CXX_STANDARD 14)

#add header files to IDE project (no target for them)
FILE(GLOB_RECURSE myHeaderFiles "${CMAKE_CURRENT_LIST_DIR}/include/*.h")
add_custom_target(headers SOURCES ${myHeaderFiles})

#templated function code
add_custom_target(AdditionalCode SOURCES src/ComputeTaskGenerator.cpp)

# Find the QtWidgets library
#find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# Populate a CMake variable with the sources
set(mandelbrot_SRCS
    src/computeddatasegment.cpp
    src/EditMenu.cpp
    src/filemenu.cpp
    src/informationdisplay.cpp
    src/main.cpp
    src/MandelbrotGuiTools.cpp
    src/mandelbrotrenderer.cpp
    src/mandelbrotwidget.cpp
    src/ParametersMenu.cpp
    src/PrecisionHandler.cpp
    src/radiointegerbutton.cpp
    src/regionattributes.cpp
    src/RendererConfig.cpp
    src/RenderHistory.cpp
    src/RenderMenu.cpp
    src/RenderParametersWidget.cpp
    src/renderthread.cpp
    src/PrecisionHandler.cpp
    src/renderthreadmediator.cpp
    src/renderworker.cpp
    src/settingshandler.cpp
    src/threadiconmap.cpp
    src/toolsmenu.cpp
    src/toolsoptionswidget.cpp
    src/windowmenu.cpp
    src/windowthreadinfo.cpp
    src/windowthreadinfokey.cpp
    src/workerthreaddata.cpp
	mandelbrotresources.qrc
	mandelbrot.rc
)
# Tell CMake to create the mandelbrot executable
add_executable(mandelbrot WIN32 ${mandelbrot_SRCS} ${myHeaderFiles} mandelbrot.rc)
# Use the Widgets module from Qt 5
target_link_libraries(mandelbrot Qt5::Widgets)

if (UNIX)
target_link_libraries(mandelbrot -lpthread -lquadmath)
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
