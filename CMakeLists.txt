cmake_minimum_required(VERSION 2.8)

project(franca)


set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -g0")

include_directories(/usr/include/python2.7)


add_library(franca SHARED
   parser.cpp
   builder.cpp
   model.cpp   
   sorter.cpp  
   depend.cpp
)


add_executable(tester
   testmodel.cpp
)
target_link_libraries(tester franca)
