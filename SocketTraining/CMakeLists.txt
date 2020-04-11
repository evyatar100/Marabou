cmake_minimum_required(VERSION 3.15)
project(SocketTraining)

set(CMAKE_CXX_STANDARD 14)

add_executable(SocketTraining main.cpp ../src/engine/TensorFlowSocket.cpp ../src/engine/TensorFlowSocket.h)