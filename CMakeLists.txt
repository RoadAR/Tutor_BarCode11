cmake_minimum_required(VERSION 3.4.1)

project(BarCode11)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


set(SRC_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/BarCode11")
set(SRC_BARCODE_FOLDER "${SRC_FOLDER}/BarCode")


file(GLOB SRC_BAR_CODE 
  "${SRC_BARCODE_FOLDER}/**.cpp"
)

add_library(barcode ${SRC_BAR_CODE})

target_include_directories(barcode
  PUBLIC
  ${SRC_BARCODE_FOLDER} 
  ${OpenCV_DIRECTORIES})


#### Main ####

add_executable(main "${SRC_FOLDER}/main.cpp")

target_link_libraries(main ${OpenCV_LIBS} barcode)

