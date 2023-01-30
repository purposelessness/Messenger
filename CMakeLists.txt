cmake_minimum_required(VERSION 3.24)
project(Messenger)

set(CMAKE_CXX_STANDARD 20)

add_executable(Messenger main.cpp Queue.h Net/NetService.cpp Net/NetService.h Message.cpp Message.h MessageVisitor.h Interface/InterfaceService.cpp Interface/InterfaceService.h)
