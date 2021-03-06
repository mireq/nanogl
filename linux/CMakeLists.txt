cmake_minimum_required(VERSION 3.5)

project(linux)
include(utilities.cmake)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../simulator/src ${CMAKE_CURRENT_BINARY_DIR}/simulator)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/font_render/ ${CMAKE_CURRENT_BINARY_DIR}/font_render)

include_directories(
	${ESP_SIMULATOR_INCLUDE_DIRECTORIES}
	"${CMAKE_SOURCE_DIR}/../components/buffer/include/"
	"${CMAKE_SOURCE_DIR}/../components/http_header_parser/include/"
	"${CMAKE_SOURCE_DIR}/../components/http_server/include/"
	"${CMAKE_SOURCE_DIR}/../components/player/include/"
	"${CMAKE_SOURCE_DIR}/../components/mad/include/"
	"${CMAKE_SOURCE_DIR}/../components/"
	"${CMAKE_SOURCE_DIR}/../components/nanogl/include/"
	"${CMAKE_SOURCE_DIR}/../components/font_render/include/"
	"${CMAKE_SOURCE_DIR}/../components/font_render/freetype2/include/"
	"${CMAKE_SOURCE_DIR}/../main/"
	"$ENV{IDF_PATH}/components/log/include/"
	"$ENV{IDF_PATH}/components/esp_common/include/"
	"$ENV{IDF_PATH}/components/esp_event/include/"
	"$ENV{IDF_PATH}/components/"
	"${CMAKE_CURRENT_BINARY_DIR}/../../../config/"
)

add_executable(
	${PROJECT_NAME}
	"init.c"
	"display.c"
	"${CMAKE_CURRENT_BINARY_DIR}/Ubuntu-R.ttf.S"
	"${CMAKE_SOURCE_DIR}/../main/gui.c"
	"${CMAKE_SOURCE_DIR}/../main/main.c"
	"${CMAKE_SOURCE_DIR}/../components/nanogl/nanogl.c"
)

add_definitions(-D_GNU_SOURCE -DSIMULATOR -g3 -ggdb)
target_compile_definitions(freertos PUBLIC -DFREERTOS_EXTRA_CONFIG)
target_include_directories(freertos PUBLIC "${CMAKE_SOURCE_DIR}/include/")

target_link_libraries(
	${PROJECT_NAME}
	font_render
	simulator
	GL
	GLEW
	glut
	m
)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_ROOT_BUILD_DIR}")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "linux_simulator")
add_embedded_file(
	${PROJECT_NAME}
	"${CMAKE_SOURCE_DIR}/../main/Ubuntu-R.ttf"
	BINARY
)
