cmake_minimum_required(VERSION 3.24)
project(C_ProgrammEntwurf)

set(CMAKE_CXX_STANDARD 17)

add_executable(C_ProgrammEntwurf src/CTextToCPP.cpp src/CTextToEscSeq.cpp src/tst.cpp)

add_executable(tst src/tst.cpp)