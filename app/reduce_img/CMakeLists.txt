add_executable(reduce_img
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(reduce_img PRIVATE
    sung::libimgref
)
