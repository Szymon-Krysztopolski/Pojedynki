cmake_minimum_required(VERSION 3.10)
project(pojedynki)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(MPI_INCLUDE $ENV{MSMPI_INC} CACHE FILEPATH "MSMPI include directory")
set(MPI_LIB "msmpi" CACHE FILEPATH "MSMPI .lib file")
set(MPI_LIB_INCLUDE $ENV{MSMPI_LIB32} CACHE FILEPATH "MSMPI .lib directory")

add_executable(pojedynki main.cpp duel.cpp duel.h)

target_link_libraries(pojedynki PUBLIC ${MPI_LIB})
target_include_directories(pojedynki PUBLIC ${MPI_INCLUDE})
target_link_directories(pojedynki PUBLIC ${MPI_LIB_INCLUDE})
