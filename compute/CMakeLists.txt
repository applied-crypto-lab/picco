cmake_minimum_required(VERSION 3.0.0)
project(compiled_program LANGUAGES  CXX)


# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

# set(CMAKE_C_COMPILER             "/usr/bin/clang-11")
# set(CMAKE_C_COMPILER             "/usr/bin/clang")
# set(CMAKE_C_FLAGS                "-maes -mavx -mavx2 -march=native -Wall -std=c99")
# set(CMAKE_C_FLAGS_DEBUG          "-g")
# set(CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
# set(CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG")
# set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")


set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_COMPILER             "/usr/bin/clang++-11")
# set(CMAKE_CXX_COMPILER             "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS                "-maes -mavx -mavx2 -march=native -Wall")
set(CMAKE_CXX_FLAGS_DEBUG          "-g3 -Wall")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
# set(CMAKE_CXX_FLAGS_RELEASE        "-Ofast")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# set(CMAKE_BUILD_PARALLEL_LEVEL 8)



# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")   # Add more flags here
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maes -march=native")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Wall")

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CryptoPP REQUIRED)
find_package(GMP REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

# base sources (NodeNetwork, NodeConfig, SMC_Utils)
file(GLOB SOURCES 
      "smc-compute/*.cpp"
)



option(DEPLOYMENT "Deployment or benchmarking mode (default Deployment)" ON) # ON by default
option(SHAMIR "Shamir secret sharing (default ON)" ON) # ON by default
option(RSS "Replicated secret sharing (default OFF)" OFF) # OFF by default

 

if(SHAMIR)
      add_definitions(-D__SHAMIR__=1)
      add_definitions(-D__RSS__=0)
      INCLUDE_DIRECTORIES(
            "${GMP_INCLUDES}"
            "${CMAKE_SOURCE_DIR}/smc-compute"
            "${CMAKE_SOURCE_DIR}/smc-compute/shamir"
            "${CMAKE_SOURCE_DIR}/smc-compute/ops/shamir"
      )
      file(GLOB TECHNIQUE 
            "smc-compute/shamir/*.cpp"
            "smc-compute/ops/shamir/*.cpp"
      )

endif()

if(RSS)
      add_definitions(-D__RSS__=1)
      add_definitions(-D__SHAMIR__=0)
      INCLUDE_DIRECTORIES(
            "${GMP_INCLUDES}"
            "${CMAKE_SOURCE_DIR}/smc-compute"
            "${CMAKE_SOURCE_DIR}/smc-compute/rss"
            "${CMAKE_SOURCE_DIR}/smc-compute/ops/rss"
      )
      file(GLOB TECHNIQUE 
            "smc-compute/rss/*.cpp"
            "smc-compute/ops/rss/*.cpp"
      )

endif()

if(DEPLOYMENT)
      add_definitions(-D__DEPLOYMENT__=1)
else()
      add_definitions(-D__DEPLOYMENT__=0)
endif()

add_executable(compiled_program test_op.cpp ${SOURCES} ${TECHNIQUE})

target_link_libraries(compiled_program ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${CRYPTOPP_LIBRARIES}  ${GMP_LIBRARIES})

add_custom_target(clean-all
  make clean && rm -fR ${CMAKE_BINARY_DIR}/CMakeCache.txt
                       ${CMAKE_BINARY_DIR}/cmake_install.cmake
                       ${CMAKE_BINARY_DIR}/Makefile
                       ${CMAKE_BINARY_DIR}/CMakeFiles
             && rmdir  ${CMAKE_BINARY_DIR}/build
)