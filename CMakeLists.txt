cmake_minimum_required(VERSION 3.0.0)
project(dyykvdb VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(dyykvdb main.cpp DataBase.cpp DataProcessor.cpp Logger.cpp TcpWork.cpp)

target_link_libraries(dyykvdb libpthread.so)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
