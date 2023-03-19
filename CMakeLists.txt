cmake_minimum_required (VERSION 3.3.0)
cmake_policy(VERSION 3.2.2)
cmake_policy(SET CMP0054 NEW)

project(cyclone VERSION 1.0.0)

set(TARGET_NAME ${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_EXAMPLES "BUILD EXAMPLES" OFF)
option(BUILD_SHARED_LIBS "BUILD SHARED LIBS" ON)

if(NOT MACOSX AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(DARWIN YES)
elseif(NOT BSD AND ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    SET(FREEBSD YES)
elseif(NOT LINUX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(LINUX YES)
endif()

# Do not allow in source builds
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Make sure we can import out CMake functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Load in the needed CMake modules
include(CheckIncludeFiles)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(AddCCompilerFlag)
include(AddCXXCompilerFlag)
include(DetermineTargetArchitecture)
include(CMakeDependentOption)

determine_target_architecture(cyclone_ARCHITECTURE)
include(GNUInstallDirs)

# Thread Stack Size
set(cyclone_THREAD_STACK_SIZE 102400 CACHE STRING
  "The stack size in bytes for each thread created")
set_property(CACHE cyclone_THREAD_STACK_SIZE PROPERTY VALUE ${cyclone_THREAD_STACK_SIZE})
message(STATUS "Thread Stack Size - ${cyclone_THREAD_STACK_SIZE}")


# IP Version 6
option(cyclone_ENABLE_IPV6 "Enables the IP version 6 support" OFF)
message(STATUS "IP Version 6 - ${cyclone_ENABLE_IPV6}")

# ASAN in debug mode (-fsanitize=address, etc)
option(cyclone_ENABLE_ASAN "Enable ASAN in debug mode" ON)
message(STATUS "ASAN in debug mode - ${cyclone_ENABLE_ASAN}")

# zlib compression support
option(cyclone_ENABLE_ZLIB "Enables zlib compression support" OFF)
message(STATUS "zlib support - ${cyclone_ENABLE_ZLIB}")

# Enable installing cyclone executables
option(cyclone_INSTALL_EXECUTABLE "Enable installing cyclone executable" ON)
mark_as_advanced(FORCE cyclone_INSTALL_EXECUTABLE) # Advanced users can disable
message(STATUS "Executable installation - ${cyclone_INSTALL_EXECUTABLE}")

option(cyclone_ALLOW_WARNINGS "Do not stop build ifthere are warnings" ON)
message(STATUS "Build ifthere are warnings - ${cyclone_ALLOW_WARNINGS}")

if(NOT cyclone_ALLOW_WARNINGS)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(FATAL_ERROR "Cannot compile with warning as errors, until this GCC bug is solved: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431")
  endif()
endif()

# SSL support
option(cyclone_ENABLE_SSL "Enables the secure socket layer" ON)
message(STATUS "SSL support - ${cyclone_ENABLE_SSL}")

# OpenSSL 1.0 API
option(cyclone_SSL_OPENSSL_API_1_0 "Use the OpenSSL 1.0 API" OFF)
message(STATUS "Compile for OpenSSL 1.0 API - ${cyclone_SSL_OPENSSL_API_1_0}")

# OpenSSL 1.1 API
option(cyclone_SSL_OPENSSL_API_1_1 "Use the OpenSSL 1.1 API" ON)
message(STATUS "Compile for OpenSSL 1.1 API - ${cyclone_SSL_OPENSSL_API_1_1}")

# Dynamically load or link the SSL libraries
cmake_dependent_option(
  cyclone_ENABLE_SSL_DYNAMIC_LOADING "Dynamically loads the SSL library rather than linking it" ON
  cyclone_ENABLE_SSL OFF)
if(cyclone_ENABLE_SSL)
  message(STATUS "Dynamically load SSL libraries - ${cyclone_ENABLE_SSL_DYNAMIC_LOADING}")
endif()

# Link time optimization
if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
    "${BUILD_SHARED_LIBS}" STREQUAL "SHARED" OR
    ${CMAKE_CXX_COMPILER_VERSION} GREATER 5)
  option(cyclone_CXX_ENABLE_LTO "Enable link time optimization for the C++ wrapper library" ON)
else()
  option(cyclone_CXX_ENABLE_LTO "Enable link time optimization for the C++ wrapper library" OFF)
endif()

# Unix systems can define the dynamic library names to load
if(cyclone_ENABLE_SSL_DYNAMIC_LOADING AND NOT DARWIN AND UNIX)
  # SSL library name
  set(cyclone_SSL_SSL_LIB "libssl.so" CACHE STRING
    "The name of the SSL library to load")
  set_property(CACHE cyclone_SSL_SSL_LIB PROPERTY VALUE ${cyclone_SSL_SSL_LIB})
  message(STATUS "SSL Library Name - ${cyclone_SSL_SSL_LIB}")

  # Crytography library name
  set(cyclone_SSL_CRYPTO_LIB "libcrypto.so" CACHE STRING
    "The name of the SSL Cryptography library to load")
  set_property(CACHE cyclone_SSL_CRYPTO_LIB PROPERTY VALUE ${cyclone_SSL_CRYPTO_LIB})
  message(STATUS "SSL Cryptography Library Name - ${cyclone_SSL_CRYPTO_LIB}")
endif()

# The C and C++ standards to use
set(cyclone_C_STANDARD auto CACHE STRING
  "The C standard to use; auto determines the latest supported by the compiler")
set_property(CACHE cyclone_C_STANDARD PROPERTY STRINGS auto c11 c99 c89)
set(cyclone_CXX_STANDARD auto CACHE STRING
  "The C++ standard to use; auto determines the latest supported by the compiler")
set_property(CACHE cyclone_CXX_STANDARD PROPERTY STRINGS auto c++14 c++11 c++98)

# Configure the linker
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  find_program(GCC_AR gcc-ar)
  if(GCC_AR)
    set(CMAKE_AR ${GCC_AR})
  endif()
  find_program(GCC_RANLIB gcc-ranlib)
  if(GCC_RANLIB)
    set(CMAKE_RANLIB ${GCC_RANLIB})
  endif()
endif()

# Configure the C compiler
message(STATUS "Configuring C Compiler")
if("${cyclone_C_STANDARD}" STREQUAL "auto")
  add_c_compiler_flag(-std=c11)
  if(NOT HAVE_C_FLAG_STD_C11)
    add_c_compiler_flag(-std=c99)
    if(NOT HAVE_C_FLAG_STD_C99)
      add_c_compiler_flag(-std=c89)
    endif()
  endif()
else()
  add_c_compiler_flag(-std=${cyclone_C_STANDARD})
endif()

if(NOT cyclone_ALLOW_WARNINGS)
  add_c_compiler_flag(-Werror)
  add_c_compiler_flag(/WX)
endif()
if(${cyclone_ENABLE_ASAN})
  add_c_compiler_flag(-fsanitize=undefined DEBUG)
  add_c_compiler_flag(-fsanitize=address DEBUG)
if(HAVE_C_FLAG_FSANITIZE_ADDRESS)
  add_c_compiler_flag(-static-asan DEBUG)
endif()
endif()

# Coverage build type
set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING
    "Flags used by the C compiler during coverage builds."
    FORCE)
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING
    "Flags used for linking binaries during coverage builds."
    FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING
    "Flags used by the shared libraries linker during coverage builds."
    FORCE)
mark_as_advanced(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage."
    FORCE)
add_c_compiler_flag(--coverage COVERAGE)

# Configure the C++ compiler
if("${cyclone_CXX_STANDARD}" STREQUAL "auto")
  add_cxx_compiler_flag(-std=c++14)
  if(NOT HAVE_CXX_FLAG_STD_CXX14)
    add_cxx_compiler_flag(-std=c++11)
    if(NOT HAVE_CXX_FLAG_STD_CXX11)
      add_cxx_compiler_flag(-std=c++98)
    endif()
  endif()
else()
  add_cxx_compiler_flag(-std=${cyclone_CXX_STANDARD})
endif()

add_cxx_compiler_flag(-Wall)
add_cxx_compiler_flag(-Wextra)
add_cxx_compiler_flag(-Wshadow)
add_cxx_compiler_flag(-Wmissing-prototypes)
add_cxx_compiler_flag(-Weverything)
add_cxx_compiler_flag(/W4)
add_cxx_compiler_flag(-Wno-padded)
add_cxx_compiler_flag(/Wd4820) # padding
add_cxx_compiler_flag(-Wno-unused-macros)
add_cxx_compiler_flag(-Wno-format-nonliteral)
if(NOT cyclone_ALLOW_WARNINGS)
  add_cxx_compiler_flag(-Werror)
  add_cxx_compiler_flag(/WX)
endif()

add_cxx_compiler_flag(-pedantic-errors)
add_cxx_compiler_flag(-fvisibility=hidden)
add_cxx_compiler_flag(-fstack-protector-strong RELEASE)

if(${cyclone_CXX_ENABLE_LTO})
  add_cxx_compiler_flag(-flto RELEASE)
endif()
if(${cyclone_ENABLE_ASAN})
  add_cxx_compiler_flag(-fsanitize=undefined DEBUG)
  add_cxx_compiler_flag(-fsanitize=address DEBUG)
  if(HAVE_CXX_FLAG_FSANITIZE_ADDRESS)
    add_cxx_compiler_flag(-static-asan DEBUG)
  endif()
endif()

add_cxx_compiler_flag(-fstack-protector-all DEBUG)
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING
    "Flags used by the C++ compiler during coverage builds."
    FORCE)
add_cxx_compiler_flag(--coverage COVERAGE)

if(NOT ZEPHYR)
  #Warnings: enable everything
  add_c_compiler_flag(-Wall)
  add_c_compiler_flag(-Wextra)
  add_c_compiler_flag(-Wshadow)
  add_c_compiler_flag(-Wconversion)
  add_c_compiler_flag(-Wmissing-prototypes)
  add_c_compiler_flag(-Weverything)
  add_c_compiler_flag(-Wparentheses)
  add_c_compiler_flag(/W4) # VisualStudio highest warning level

  #Warnings: Disable some warnings
  add_c_compiler_flag(-Wno-padded) # padding in structures by compiler
  add_c_compiler_flag(-Wno-unused-macros) # so what?
  Check_C_Compiler_Flag( HAVE_NO_RESERVED_ID_MACROS -Wno-reserved-id-macros)
  if(HAVE_NO_RESERVED_ID_MACROS)
    add_c_compiler_flag(-Wno-reserved-id-macros) # for system headers
  endif(HAVE_NO_RESERVED_ID_MACROS)
  add_c_compiler_flag(-Wno-format-nonliteral) # printf(myFormatStringVar, ...)
  add_c_compiler_flag(-Wno-cast-qual) # const cast
  add_c_compiler_flag(/Wd4820) # padding

  add_c_compiler_flag(-pedantic-errors)
  add_c_compiler_flag(-fvisibility=hidden)
  add_c_compiler_flag(-fstack-protector-strong RELEASE)
  add_c_compiler_flag(-fstack-protector-all DEBUG)
else()
  # This policy is needed to override options with variables
  cmake_policy(SET CMP0077 NEW)

  # Configure what you need/support in Zephyr
  set(cyclone_SERVE_NO_FILESYSTEMS ON)
  set(cyclone_ENABLE_SSL OFF)
  set(cyclone_ENABLE_SSL_DYNAMIC_LOADING OFF)

  set(cyclone_ENABLE_ASAN OFF)
  set(cyclone_INSTALL_EXECUTABLE OFF)

  set(cyclone_THREAD_STACK_SIZE 0)

  set(BUILD_SHARED_LIBS ON)

  add_definitions(-DMG_EXTERNAL_FUNCTION_mg_cry_internal_impl)
  add_definitions(-DMG_EXTERNAL_FUNCTION_log_access)

  add_definitions(-DNO_ALTERNATIVE_QUEUE)
  add_definitions(-DZEPHYR_VERSION=${KERNEL_VERSION_STRING})

  zephyr_interface_library_named(cyclone)

  target_include_directories(cyclone INTERFACE src)
  target_include_directories(cyclone INTERFACE include)
  target_include_directories(cyclone INTERFACE ${CMAKE_SOURCE_DIR}/src)

  zephyr_include_directories(include)

  zephyr_library()
  zephyr_library_sources(
    src/civetweb.c
  )

  zephyr_library_link_libraries(cyclone)
  target_link_libraries(cyclone INTERFACE zephyr_interface)
endif()

# Set up the definitions
option(cyclone_ENABLE_DEBUG_TOOLS "For Debug builds enable verbose logging and assertions" ON)
add_definitions(-DUSE_WEBSOCKET)
if(${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug")
  if(cyclone_ENABLE_DEBUG_TOOLS)
    add_definitions(-DDEBUG)
  endif()
  add_definitions(-O0)
  add_definitions(-g)
endif()
if(cyclone_ENABLE_IPV6)
  add_definitions(-DUSE_IPV6)
endif()
if(cyclone_SERVE_NO_FILESYSTEMS)
  add_definitions(-DNO_FILESYSTEMS)
endif()
if(cyclone_ENABLE_ZLIB)
  add_definitions(-DUSE_ZLIB)
endif()
if(NOT cyclone_ENABLE_SSL)
  add_definitions(-DNO_SSL)
elseif(NOT cyclone_ENABLE_SSL_DYNAMIC_LOADING)
  add_definitions(-DNO_SSL_DL)
else()
  if(cyclone_SSL_SSL_LIB)
    add_definitions(-DSSL_LIB="${cyclone_SSL_SSL_LIB}")
  endif()
  if(cyclone_SSL_CRYPTO_LIB)
    add_definitions(-DCRYPTO_LIB="${cyclone_SSL_CRYPTO_LIB}")
  endif()
endif()

if(cyclone_SSL_OPENSSL_API_1_0)
  add_definitions(-DOPENSSL_API_1_0)
endif()
if(cyclone_SSL_OPENSSL_API_1_1)
  add_definitions(-DOPENSSL_API_1_1)
endif()
if(cyclone_SSL_OPENSSL_API_1_0 AND cyclone_SSL_OPENSSL_API_1_1)
  message(FATAL_ERROR "Multiple SSL API versions defined")
endif()

add_definitions(-DUSE_STACK_SIZE=${cyclone_THREAD_STACK_SIZE})

if(ZEPHYR)
  return()
endif()

# --------------------------------------------------------------
#                       build
# --------------------------------------------------------------
set(LIB_TYPE "STATIC")
if(BUILD_SHARED_LIBS)
  set(LIB_TYPE "SHARED")
endif()

include_directories(
  include
)

file(GLOB CYCLONE_SRCS
  "src/civetweb/*.c"
  "src/civetweb/*.cc"
  "src/*.cc"
)

add_library(${TARGET_NAME} ${LIB_TYPE} ${CYCLONE_SRCS})

find_package(WinSock)
if(WINSOCK_FOUND)
  target_link_libraries(${TARGET_NAME} WINSOCK::WINSOCK)
endif()

find_package(Threads)
target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})

find_package(LibRt)
if(LIBRT_FOUND)
  target_link_libraries(${TARGET_NAME} LIBRT::LIBRT)
endif()

if(cyclone_ENABLE_SSL)
  if(cyclone_ENABLE_SSL_DYNAMIC_LOADING)
    find_package(LibDl)
    if(LIBDL_FOUND)
      target_link_libraries(${TARGET_NAME} -ldl)
    endif()
  else()
    find_package(OpenSSL)
    include_directories(${OPENSSL_INCLUDE_DIR})
    message(STATUS "OpenSSL include directory: ${OPENSSL_INCLUDE_DIR}")
    target_link_libraries(${TARGET_NAME} ${OPENSSL_LIBRARIES})
  endif()
endif()

if(cyclone_ENABLE_ZLIB)
  find_package(ZLIB)
  include_directories(${ZLIB_INCLUDE_DIRS})
  target_link_libraries(${TARGET_NAME} ${ZLIB_LIBRARIES})
endif()

set_target_properties(${TARGET_NAME} PROPERTIES
  OUTPUT_NAME ${TARGET_NAME} 
  EXPORT_NAME ${TARGET_NAME}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION}
)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(${TARGET_NAME} PRIVATE cyclone_DLL_EXPORTS INTERFACE cyclone_DLL_IMPORTS)
endif()

install(
  TARGETS ${TARGET_NAME}
  EXPORT ${TARGET_NAME}-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT ${TARGET_NAME}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.inl"
)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(BUILD_EXAMPLES)

include(CTest)
include(CMakePackageConfigHelpers)

install(
  EXPORT ${TARGET_NAME}-targets
  NAMESPACE ${TARGET_NAME}::
  FILE ${PROJECT_NAME}-targets.cmake
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  COMPONENT ${TARGET_NAME}-cmake-config
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${TARGET_NAME}-config.cmake.in"
  ${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET_NAME}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

configure_file(
  cmake/${PROJECT_NAME}.pc.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc
  @ONLY
)

install(
  FILES
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/pkgconfig"
)

write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindLibDl.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindLibRt.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindWinSock.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  COMPONENT ${TARGET_NAME}-cmake-config
)
