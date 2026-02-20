cmake_minimum_required(VERSION 3.0.0)
project(compiled_program LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-maes -mavx -mavx2 -march=native -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Wall")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(OMPDIR "/insert/ompi/install/dir/here")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      message("using clang")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      message("using gcc")
endif()

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(OMPI)
find_package(GMP REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

# base sources (NodeNetwork, NodeConfig, SMC_Utils)
file(GLOB SOURCES
      "smc-compute/*.cpp"
)

option(DEPLOYMENT "Deployment or benchmarking mode (default=Deployment)" ON) # ON by default
option(SHAMIR "Shamir secret sharing (default=ON)" ON) # ON by default
option(RSS "Replicated secret sharing (default=OFF)" OFF) # OFF by default

# RSS and SHAMIR are mutually exclusive - if RSS is enabled, disable SHAMIR
if(RSS)
      set(SHAMIR OFF)
endif()

# Exclude RSS_testing.cpp when building Shamir
if(SHAMIR)
      list(FILTER SOURCES EXCLUDE REGEX ".*RSS_testing\\.cpp$")
endif()

if(OMPI_FOUND)
      set(OMPI_INC "${OMPI_INCLUDES}")
endif()

INCLUDE_DIRECTORIES(
      "${GMP_INCLUDES}"
      "${CMAKE_SOURCE_DIR}/smc-compute"
      ${OMPI_INC}
)

if(SHAMIR)
      set(TECHNIQUE_DIR "shamir")
      add_definitions(-D__SHAMIR__=1)
      add_definitions(-D__RSS__=0)
endif()


# used for setting the data types for RSS
option(USE_32 "32 bit or less ring sizes" ON)
option(USE_64 "33 to 64 bit rings" OFF)
option(USE_128 "65 to 128 bit rings" OFF)

if(RSS)
      set(TECHNIQUE_DIR "rss")
      add_definitions(-D__RSS__=1)
      add_definitions(-D__SHAMIR__=0)

      if(USE_128)
            add_definitions(-D__RSS_32__=0)
            add_definitions(-D__RSS_64__=0)
            add_definitions(-D__RSS_128__=1)
      elseif(USE_64)
            add_definitions(-D__RSS_32__=0)
            add_definitions(-D__RSS_64__=1)
            add_definitions(-D__RSS_128__=0)
      elseif(USE_32)
            add_definitions(-D__RSS_32__=1)
            add_definitions(-D__RSS_64__=0)
            add_definitions(-D__RSS_128__=0)
      endif()
      unset(USE_128 CACHE)
      unset(USE_64 CACHE)
      unset(USE_32 CACHE)
endif()

INCLUDE_DIRECTORIES(
      "${CMAKE_SOURCE_DIR}/smc-compute/${TECHNIQUE_DIR}"
      "${CMAKE_SOURCE_DIR}/smc-compute/ops/${TECHNIQUE_DIR}"
)

file(GLOB TECHNIQUE
      "smc-compute/${TECHNIQUE_DIR}/*.cpp"
      "smc-compute/ops/${TECHNIQUE_DIR}/*.cpp"
)

if(DEPLOYMENT)
      add_definitions(-D__DEPLOYMENT__=1)
else()
      add_definitions(-D__DEPLOYMENT__=0)
endif()

set(LIBS
      ${CMAKE_THREAD_LIBS_INIT}
      ${OPENSSL_LIBRARIES}
      # ${CRYPTOPP_LIBRARIES}
      ${GMP_LIBRARIES}
)

if(OMPI_FOUND)
      list(APPEND LIBS ${OMPI_LIBRARIES})
endif()

# Only build rss_main when RSS is enabled
if(RSS)
    add_executable(rss_main rss_main.cpp ${SOURCES} ${TECHNIQUE})
    target_link_libraries(rss_main ${LIBS})
endif()

add_custom_target(clean-all
      make clean && rm -fR ${CMAKE_BINARY_DIR}/CMakeCache.txt
      ${CMAKE_BINARY_DIR}/cmake_install.cmake
      ${CMAKE_BINARY_DIR}/Makefile
      ${CMAKE_BINARY_DIR}/CMakeFiles
      && rmdir ${CMAKE_BINARY_DIR}/build
)