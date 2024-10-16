add_executable(reduce_img_tui
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(reduce_img_tui PRIVATE
    ftxui::dom
    ftxui::screen
    ftxui::component
    sung::libimgref
)
