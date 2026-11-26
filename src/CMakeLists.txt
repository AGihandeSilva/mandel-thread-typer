cmake_minimum_required(VERSION 3.30.9)

project(mandelbrot)

set(FETCHCONTENT_QUIET OFF)
set(USE_GRPC OFF)
set(USE_TEST_DEFINES ON)
set(MANDELBROT_USE_SFINAE OFF)
set(MANDELBROT_ENABLE_HTTP_SERVER_CODE OFF)
set(DEBUG_RAW_DATA OFF)
set(ALLOCATE_EXTRA_CHARACTERS_FOR_STANDARD_VALUE ON)

if (USE_GRPC)
include(../grpc_cmake/common.cmake)
add_definitions(-DMANDEL_USE_GRPC)
endif (USE_GRPC)

if (MANDELBROT_USE_SFINAE)
add_definitions(-DMANDELBROT_USE_SFINAE)
endif(MANDELBROT_USE_SFINAE)

if (MANDELBROT_ENABLE_HTTP_SERVER_CODE)
add_definitions(-DMANDELBROT_ENABLE_HTTP_SERVER_CODE)
endif(MANDELBROT_ENABLE_HTTP_SERVER_CODE)

if (DEBUG_RAW_DATA)
add_definitions(-DDEBUG_RAW_DATA)
endif(DEBUG_RAW_DATA)

if (ALLOCATE_EXTRA_CHARACTERS_FOR_STANDARD_VALUE)
#workaround for an apparent issue in the calculated number of digits returned by boost for float_oct 
add_definitions(-DALLOCATE_EXTRA_CHARACTERS_FOR_STANDARD_VALUE)
endif(ALLOCATE_EXTRA_CHARACTERS_FOR_STANDARD_VALUE)


set(cmake_include_path ${cmake_include_path})
set(cmake_module_path ${cmake_module_path})

include(FetchContent)


fetchcontent_declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

fetchcontent_declare(
    Boost
    GIT_REPOSITORY "https://github.com/boostorg/boost.git"
    GIT_TAG "boost-1.91.0"
    GIT_PROGRESS ON
    GIT_SHALLOW ON
    OVERRIDE_FIND_PACKAGE TRUE # needed to find correct Boost
    EXCLUDE_FROM_ALL # compile only what you need
)

fetchcontent_makeavailable(Boost)
fetchcontent_makeavailable(json)

#set up include directories
include_directories(./include)
include_directories(./src)
if (MANDELBROT_ENABLE_HTTP_SERVER_CODE)
include_directories(./include/restAPI)
endif (MANDELBROT_ENABLE_HTTP_SERVER_CODE)

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

set(CMAKE_CXX_STANDARD 20)

#add header files to IDE project (no target for them)
FILE(GLOB_RECURSE myHeaderFiles "${CMAKE_CURRENT_LIST_DIR}/include/*.h")
add_custom_target(headers SOURCES ${myHeaderFiles})

#templated function code
add_custom_target(AdditionalCode SOURCES src/ComputeTaskGenerator.cpp)

if (USE_GRPC)
#gRPC Dependencies here

# Proto files
get_filename_component(hw_proto "../grpc_protos/mandelbrot.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/mandelbrot.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/mandelbrot.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/mandelbrot.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/mandelbrot.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

#################

include(ExternalProject)

# Note: For all external projects, instead of using checked-out code, one could
# specify GIT_REPOSITORY and GIT_TAG to have cmake download the dependency directly,
# without needing to add a submodule to your project.

# Builds absl project from the git submodule.
ExternalProject_Add(absl
  PREFIX absl
  SOURCE_DIR "./../third_party/abseil-cpp"
  CMAKE_CACHE_ARGS
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=TRUE
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/absl
        -DCMAKE_CXX_STANDARD:STRING=17
)

# Builds utf8_range project from the git submodule.
ExternalProject_Add(utf8_range
  PREFIX utf8_range
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/utf8_range"
  CMAKE_CACHE_ARGS
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=TRUE
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/utf8_range
        -DCMAKE_CXX_STANDARD:STRING=17
        -Dutf8_range_ENABLE_TESTS:BOOL=OFF
        -Dprotobuf_VERSION:STRING=0.0.1
        -Dabsl_DIR:STRING=${CMAKE_CURRENT_BINARY_DIR}/absl/lib/cmake/absl
  DEPENDS absl
)

# Builds c-ares project from the git submodule.
ExternalProject_Add(c-ares
  PREFIX c-ares
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cares/cares"
  CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/c-ares
        -DCARES_SHARED:BOOL=OFF
        -DCARES_STATIC:BOOL=ON
        -DCARES_STATIC_PIC:BOOL=ON
)

# Builds protobuf project from the git submodule.
ExternalProject_Add(protobuf
  PREFIX protobuf
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf"
  CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/protobuf
        -DCMAKE_CXX_STANDARD:STRING=17
        -Dprotobuf_BUILD_TESTS:BOOL=OFF
        -Dprotobuf_WITH_ZLIB:BOOL=OFF
        -Dprotobuf_ABSL_PROVIDER:STRING=package
        -Dabsl_DIR:STRING=${CMAKE_CURRENT_BINARY_DIR}/absl/lib/cmake/absl
        -Dutf8_range_DIR:STRING=${CMAKE_CURRENT_BINARY_DIR}/utf8_range/lib/cmake/utf8_range
        -Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
  DEPENDS absl utf8_range
)

# Builds re2 project from the git submodule.
ExternalProject_Add(re2
  PREFIX re2
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/re2"
  CMAKE_CACHE_ARGS
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=TRUE
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/re2
        -DCMAKE_CXX_STANDARD:STRING=17
)

# Builds zlib project from the git submodule.
ExternalProject_Add(zlib
  PREFIX zlib
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zlib"
  CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/zlib
)

# if OPENSSL_ROOT_DIR is set, propagate that hint path to the external projects with OpenSSL dependency.
set(_CMAKE_ARGS_OPENSSL_ROOT_DIR "")
if (OPENSSL_ROOT_DIR)
  set(_CMAKE_ARGS_OPENSSL_ROOT_DIR "-DOPENSSL_ROOT_DIR:PATH=${OPENSSL_ROOT_DIR}")
endif()

# Builds gRPC based on locally checked-out sources and set arguments so that all the dependencies
# are correctly located.
ExternalProject_Add(grpc
  PREFIX grpc
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/grpc"
  CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/grpc
        -DCMAKE_CXX_STANDARD:STRING=17
        -DgRPC_INSTALL:BOOL=ON
        -DgRPC_BUILD_TESTS:BOOL=OFF
        -DgRPC_BUILD_MSVC_MP_COUNT:STRING=-1
        -Dutf8_range_DIR:STRING=${CMAKE_CURRENT_BINARY_DIR}/utf8_range/lib/cmake/utf8_range
        -DgRPC_PROTOBUF_PROVIDER:STRING=package
        -DProtobuf_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/protobuf/lib/cmake/protobuf
        -DgRPC_RE2_PROVIDER:STRING=package
        -Dre2_DIR:STRING=${CMAKE_CURRENT_BINARY_DIR}/re2/lib/cmake/re2
        -DgRPC_ZLIB_PROVIDER:STRING=package
        -DZLIB_ROOT:STRING=${CMAKE_CURRENT_BINARY_DIR}/zlib
        -DgRPC_ABSL_PROVIDER:STRING=package
        -Dabsl_DIR:STRING=${CMAKE_CURRENT_BINARY_DIR}/absl/lib/cmake/absl
        -DgRPC_CARES_PROVIDER:STRING=package
        -Dc-ares_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/c-ares/lib/cmake/c-ares
        -DgRPC_SSL_PROVIDER:STRING=package
        ${_CMAKE_ARGS_OPENSSL_ROOT_DIR}
  DEPENDS c-ares protobuf re2 zlib absl
)
endif (USE_GRPC)

################## end of gRPC dependencies
# Find the QtWidgets library
#find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

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

if (USE_GRPC)
set(mandelbrot_grpc_SRCS
    src/RPCHandler.cpp
    src/GRPCHandler.cpp
	)
else()
set(mandelbrot_grpc_SRCS
)
endif (USE_GRPC)

if (MANDELBROT_ENABLE_HTTP_SERVER_CODE)
set(mandelbrot_HTTP_CODE_SRCS
    src/HTTPServer.cpp
)
else()
set(mandelbrot_HTTP_CODE_SRCS
)
endif(MANDELBROT_ENABLE_HTTP_SERVER_CODE)

# Tell CMake to create the mandelbrot executable
add_executable(mandelbrot WIN32 ${mandelbrot_SRCS} ${mandelbrot_grpc_SRCS} ${mandelbrot_HTTP_CODE_SRCS} ${myHeaderFiles} mandelbrot.rc)
# Use the Widgets module from Qt 6

target_include_directories(mandelbrot PRIVATE ./_deps/json-src/include)

#target_include_directories(mandelbrot PRIVATE ./_deps/absl-src)
##### TODO tidy up the include paths###
if (USE_GRPC)
#grpc
target_include_directories(mandelbrot PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/_deps/absl-src)

target_include_directories(mandelbrot PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/_deps/grpc-src/include)
target_include_directories(mandelbrot PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/_deps/protobuf-src/src)




include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories(${CMAKE_CURRENT_BINARY_DIR}/_deps/protobuf-src/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/_deps/absl-src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/utf8_range)
include_directories("C:/Users/gdesi/cmake/include")

# hw_grpc_proto
add_library(mandelbrot_lib
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})

find_package(Threads REQUIRED)
find_package(gRPC CONFIG REQUIRED)
endif (USE_GRPC)

target_link_libraries(mandelbrot 
PRIVATE Qt6::Widgets 
Boost::multiprecision 
Boost::asio
Boost::beast
)

if (USE_GRPC)
target_link_libraries(mandelbrot 
PRIVATE
gRPC::grpc++
absl::flags
absl::flags_parse
absl::log_initialize
absl::strings
${_REFLECTION}
${_GRPC_GRPCPP}
${_PROTOBUF_LIBPROTOBUF}
mandelbrot_lib
)
endif (USE_GRPC)


if (UNIX)
target_link_libraries(mandelbrot PRIVATE pthread quadmath)
endif (UNIX)

#add boost for extended type support (as desired)
#add the required boost header files with the 
#parent 'boost' directory at the same level as this project directory
#and edit the line below to enable/disable the use of these types
set (USE_BOOST_MULTIPRECISION_128_OVER_GCC_FLOAT_128 OFF)
include_directories(../)

if (${USE_BOOST_MULTIPRECISION_128_OVER_GCC_FLOAT_128})

add_definitions(-DPREFER_BOOST_MULTIPRECISION_FLOAT_128=1)
endif()
