cmake_minimum_required(VERSION 3.24)
project(C_ProgrammEntwurf)

set(CMAKE_CXX_STANDARD 17)

add_executable(C_ProjektArbeit src/gentxtsrccode.cpp hdr/CTextToCPP.h hdr/Tags.h)

add_executable(tst src/tst.cpp)