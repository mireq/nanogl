cmake_minimum_required(VERSION 3.5)

project(linux)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../simulator/src ${CMAKE_CURRENT_BINARY_DIR}/simulator)

include_directories(
	${ESP_SIMULATOR_INCLUDE_DIRECTORIES}
	"${CMAKE_SOURCE_DIR}/../components/buffer/include/"
	"${CMAKE_SOURCE_DIR}/../components/http_header_parser/include/"
	"${CMAKE_SOURCE_DIR}/../components/http_server/include/"
	"${CMAKE_SOURCE_DIR}/../components/player/include/"
	"${CMAKE_SOURCE_DIR}/../components/mad/include/"
	"${CMAKE_SOURCE_DIR}/../components/"
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
	"${CMAKE_SOURCE_DIR}/../main/main.c"
)

add_definitions(-D_GNU_SOURCE -DSIMULATOR -g3 -ggdb)
target_compile_definitions(freertos PUBLIC -DFREERTOS_EXTRA_CONFIG)
target_include_directories(freertos PUBLIC "${CMAKE_SOURCE_DIR}/include/")

target_link_libraries(
	${PROJECT_NAME}
	simulator
	GL
	GLEW
	glut
	m
)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_ROOT_BUILD_DIR}")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "linux_simulator")
