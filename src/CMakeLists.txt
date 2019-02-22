cmake_minimum_required(VERSION 3.12)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

include_directories("../include")
add_executable(1copy main.cpp onecopy.cpp OptionParser.cpp)
set_property(TARGET 1copy PROPERTY CXX_STANDARD 11)