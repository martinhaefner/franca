cmake_minimum_required(VERSION 2.8)

project(franca)


set(CMAKE_VERBOSE_MAKEFILE true)

add_compile_options(-std=c++11 -pthread)

include_directories(/usr/include/python2.7)


add_library(franca SHARED
   parser.cpp
   builder.cpp
   model.cpp   
   sorter.cpp  
   depend.cpp
)

set_source_files_properties(parser.cpp PROPERTIES COMPILE_FLAGS -g0)
set_source_files_properties(builder.cpp PROPERTIES COMPILE_FLAGS -g)


add_executable(tester
   testmodel.cpp
)
target_link_libraries(tester franca)
