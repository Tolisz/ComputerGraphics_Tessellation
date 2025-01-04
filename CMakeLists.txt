cmake_minimum_required(VERSION 3.20)

project(ComputerGraphics_Tessellation VERSION 0.1 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)

option(TESSELLATION_EMBEDDED_SHADERS "All shaders will be embedded into program binary" ON)
option(TESSELLATION_SHOW_DEBUG_CONSOLE "If On should open only window without a console" OFF)

add_subdirectory(externals)
add_subdirectory(utils)

set(TESSELLATION_LINK_LIBS 	PUBLIC glm
              				PUBLIC glfw
              				PUBLIC glad
							PUBLIC imgui
							PUBLIC gli)

set(TESSELLATION_EXECUTABLE_NAME tessellation)
file(GLOB TESSELLATION_SOURCE_FILES source/*.cpp)

add_executable(${TESSELLATION_EXECUTABLE_NAME} ${TESSELLATION_SOURCE_FILES})
target_link_libraries(${TESSELLATION_EXECUTABLE_NAME} ${TESSELLATION_LINK_LIBS})

if (TESSELLATION_EMBEDDED_SHADERS)
	message("-- TESSELLATION: TESSELLATION_EMBEDDED_SHADERS option is ON")

	target_compile_definitions(${TESSELLATION_EXECUTABLE_NAME} PRIVATE "EMBEDDED_SHADERS")
    shaders_to_embedded_hpp(${TESSELLATION_EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/shaders)
else()
	message("-- TESSELLATION: TESSELLATION_EMBEDDED_SHADERS option is OFF")
endif()

if (MSVC)
	target_compile_options(${TESSELLATION_EXECUTABLE_NAME} PRIVATE /W4)

	if (NOT TESSELLATION_SHOW_DEBUG_CONSOLE)

		message("-- TESSELLATION: TESSELLATION_SHOW_DEBUG_CONSOLE option is OFF")
		set_target_properties(${TESSELLATION_EXECUTABLE_NAME} PROPERTIES 
			LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
	else() 
		message("-- TESSELLATION: TESSELLATION_SHOW_DEBUG_CONSOLE option is ON")
	endif()

else()
	target_compile_options(${TESSELLATION_EXECUTABLE_NAME} PRIVATE -Wall -Wextra -Wpedantic)

	# Is is not that simple ....
	if (NOT TESSELLATION_SHOW_DEBUG_CONSOLE)
		message("-- TESSELLATION: TESSELLATION_SHOW_DEBUG_CONSOLE option is OFF")
	else() 
		message("-- TESSELLATION: TESSELLATION_SHOW_DEBUG_CONSOLE option is ON")
	endif()
endif()