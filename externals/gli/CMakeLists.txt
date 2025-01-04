set(GLI_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/gli)

add_library(gli INTERFACE)
target_include_directories(gli INTERFACE ${GLI_PATHS})