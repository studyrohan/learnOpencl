# CMakeList.txt: learnOpencl 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
project("OpenclTest")
set(3rd ${CMAKE_CURRENT_SOURCE_DIR}/../3rd)

add_executable (${PROJECT_NAME} "learnOpencl.cpp" "opencltest.cpp" "learnOpencl.h")
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${3rd}/opencl/include
    )


target_link_libraries(
        ${PROJECT_NAME} PRIVATE
        ${3rd}/opencl/lib/OpenCL.lib
    )


# TODO: 如有需要，请添加测试并安装目标。
