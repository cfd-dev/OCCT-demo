cmake_minimum_required(VERSION 3.10)

project(DEMO_OCCT LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# 设置VTK依赖库的路径
# 使用绝对路径指定VTK和OpenCASCADE的安装目录
set(VTK_DIR "C:/software/VTK/")
find_package(VTK REQUIRED HINTS "${VTK_DIR}/lib/cmake")
if(NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found. Please check the VTK_DIR.")
endif()

# 设置OpenCASCADE依赖库的路径
set(OCC_DIR "C:/software/occt_vc14-64/")
find_package(OpenCASCADE REQUIRED HINTS "${OCC_DIR}/cmake")
if(NOT OpenCASCADE_FOUND)
    message(FATAL_ERROR "OpenCASCADE not found. Please check the OCC_DIR.")
endif()

# 构建可执行程序
add_executable(${CMAKE_PROJECT_NAME} main.cpp)

# 设置包含目录
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${OpenCASCADE_INCLUDE_DIRS}
                                            ${VTK_INCLUDE_DIRS})

# 链接动态库
target_link_libraries(${CMAKE_PROJECT_NAME} ${OpenCASCADE_LIBRARIES} 
                                          ${VTK_LIBRARIES})