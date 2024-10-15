add_executable(reduce_img
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(reduce_img PRIVATE
    OpenImageIO::OpenImageIO
)
target_compile_features(reduce_img PRIVATE cxx_std_20)
