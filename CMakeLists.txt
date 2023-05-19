cmake_minimum_required(VERSION 3.18)
project(pylibnfc LANGUAGES CXX VERSION "1.0")

find_package(Python3 COMPONENTS Interpreter Development)
message(STATUS "Python: ${Python3_INCLUDE_DIRS}")
find_package(pybind11 CONFIG REQUIRED)
find_library(NFC_LIBRARY NAMES nfc REQUIRED)
message(STATUS "Found libnfc: ${NFC_LIBRARY}")


include_directories("${Python3_INCLUDE_DIRS}")

pybind11_add_module(${PROJECT_NAME} "pylibnfc.cpp")
  set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${NFC_LIBRARY}")

install(TARGETS ${PROJECT_NAME} DESTINATION ".")
