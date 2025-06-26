cmake_minimum_required(VERSION 3.10)

project(DEMO_OCCT LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# 构建可执行程序
set(2d_target "${CMAKE_PROJECT_NAME}_2D")
add_executable(${2d_target} main2d.cpp)

set(3d_target "${CMAKE_PROJECT_NAME}_3D")
add_executable(${3d_target} main3d.cpp)

set(parallel_target "${CMAKE_PROJECT_NAME}_PARALLEL")
add_executable(${parallel_target} parallel_projection.cpp)

# 设置VTK依赖库的路径
set(VTK_DIR "C:/software/VTK/" CACHE PATH "path to VTK library.")
find_package(VTK REQUIRED HINTS "${VTK_DIR}/lib/cmake")
if(NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found. Please check the VTK_DIR.")
else()
    message(STATUS "VTK found: ${VTK_DIR}")
    message(STATUS "VTK version: ${VTK_VERSION}")
    # message(STATUS "VTK libraries: ${VTK_LIBRARIES}")
endif()

# 设置OpenCASCADE依赖库的路径
set(OCC_DIR "C:/software/occt_vc14-64/" CACHE PATH "path to OpenCasCade library.")
find_package(OpenCASCADE REQUIRED HINTS "${OCC_DIR}/cmake")
if(NOT OpenCASCADE_FOUND)
    message(FATAL_ERROR "OpenCASCADE not found. Please check the OCC_DIR.")
else()
    message(STATUS "OpenCASCADE found: ${OCC_DIR}")
    message(STATUS "OpenCASCADE version: ${OpenCASCADE_VERSION}")
    message(STATUS "OpenCASCADE include dirs: ${OpenCASCADE_INCLUDE_DIR}")
    # message(STATUS "OpenCASCADE libraries: ${OpenCASCADE_LIBRARIES}")
endif()

#开启OPENMP编译选项
target_compile_options(${parallel_target} PRIVATE /openmp)
add_definitions(-D_OPENMP)

#设置TBB路径
set(TBB_DIR "C:/software/Intel/oneAPI/tbb/2022.1/lib/cmake/tbb" CACHE PATH "path to TBB library.")
find_package(TBB REQUIRED HINTS "${TBB_DIR}" NO_DEFAULT_PATH)
if (NOT TBB_FOUND)
    message(FATAL_ERROR "TBB library not found. Please install TBB or set the correct path.")
else()
    message(STATUS "TBB library version: ${TBB_VERSION}")
    message(STATUS "TBB library found at: ${TBB_DIR}")
endif()

#链接库和target
target_link_libraries(${2d_target} ${OpenCASCADE_LIBRARIES} ${VTK_LIBRARIES})
target_link_libraries(${3d_target} ${OpenCASCADE_LIBRARIES} ${VTK_LIBRARIES})
target_link_libraries(${parallel_target} ${OpenCASCADE_LIBRARIES} ${VTK_LIBRARIES} TBB::tbb)