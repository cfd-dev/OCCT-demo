cmake_minimum_required(VERSION 3.10)

project(DEMO_OCCT LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# 构建可执行程序
add_executable(${CMAKE_PROJECT_NAME} main.cpp)

# 设置VTK依赖库的路径
set(VTK_DIR "C:/software/VTK/")
find_package(VTK REQUIRED HINTS "${VTK_DIR}/lib/cmake")
if(NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found. Please check the VTK_DIR.")
else()
    message(STATUS "VTK found: ${VTK_DIR}")
    message(STATUS "VTK version: ${VTK_VERSION}")
    # message(STATUS "VTK libraries: ${VTK_LIBRARIES}")
endif()
target_link_libraries(${CMAKE_PROJECT_NAME} ${VTK_LIBRARIES})

# 设置OpenCASCADE依赖库的路径
set(OCC_DIR "C:/software/occt_vc14-64/")
find_package(OpenCASCADE REQUIRED HINTS "${OCC_DIR}/cmake")
if(NOT OpenCASCADE_FOUND)
    message(FATAL_ERROR "OpenCASCADE not found. Please check the OCC_DIR.")
else()
    message(STATUS "OpenCASCADE found: ${OCC_DIR}")
    message(STATUS "OpenCASCADE version: ${OpenCASCADE_VERSION}")
    message(STATUS "OpenCASCADE include dirs: ${OpenCASCADE_INCLUDE_DIR}")
    # message(STATUS "OpenCASCADE libraries: ${OpenCASCADE_LIBRARIES}")
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} ${OpenCASCADE_LIBRARIES})
