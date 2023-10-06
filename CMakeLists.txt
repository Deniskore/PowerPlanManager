cmake_minimum_required(VERSION 3.16)
project(PowerPlanManager)

message(CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_UNICODE -DUNICODE)

enable_language(RC)
set(RESOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/resources.rc)

if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /GT /GL /Gw /GF /Gy /O2")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")
  message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS_RELEASE}")
  message(STATUS "Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")
endif(MSVC)

file(GLOB SOURCE_FILES
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(PowerPlanManager WIN32
  ${SOURCE_FILES}
  ${RESOURCE_FILE})

target_link_libraries(PowerPlanManager PRIVATE pdh winmm powrprof shell32 comctl32)
