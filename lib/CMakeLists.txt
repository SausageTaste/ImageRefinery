add_library(sung_libimgref STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filesys.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/img_refinery.cpp
)
add_library(sung::libimgref ALIAS sung_libimgref)
target_include_directories(sung_libimgref PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(sung_libimgref PUBLIC
    OpenImageIO::OpenImageIO
    uni-algo::uni-algo
    sungtools::general
)
target_compile_features(sung_libimgref PUBLIC cxx_std_20)
