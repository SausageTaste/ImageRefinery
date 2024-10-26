cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(ImageRefinery)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/SungToolsCpp)

find_package(ftxui CONFIG REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)
find_package(uni-algo CONFIG REQUIRED)


add_subdirectory(lib)
add_subdirectory(app)
