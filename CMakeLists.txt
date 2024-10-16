project(LidarTools)

SET(CMAKE_BUILD_TYPE "Release")

#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -g -ggdb ")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(Eigen3 REQUIRED)
INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR})

find_package(OpenImageIO REQUIRED)
include_directories(${OPENIMAGEIO_INCLUDES})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/lib libWuLasLib_src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/LAStools/LASlib/src laslib_src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/LAStools/LASzip/src laszip_src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/IO io_src)

include_directories(${CMAKE_CURRENT_LIST_DIR}/LAStools/LASlib/inc)
include_directories(${CMAKE_CURRENT_LIST_DIR}/LAStools/LASlib/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/LAStools/LASzip/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/lib)
include_directories(${CMAKE_CURRENT_LIST_DIR}/IO)

add_library(WuLasLib SHARED ${laslib_src} ${laszip_src} ${libWuLasLib_src})
add_executable(ConvertLasImage ${CMAKE_CURRENT_LIST_DIR}/example/lidar_raster.cpp ${io_src})
add_executable(ConvertLasImageHeigth ${CMAKE_CURRENT_LIST_DIR}/example/lidar_raster_height.cpp ${io_src})

target_link_libraries(ConvertLasImage WuLasLib)
target_link_libraries(ConvertLasImage OpenImageIO)

target_link_libraries(ConvertLasImageHeigth WuLasLib)
target_link_libraries(ConvertLasImageHeigth OpenImageIO)
