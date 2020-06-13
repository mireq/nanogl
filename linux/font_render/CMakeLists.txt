cmake_minimum_required(VERSION 3.5)

project(font_render)

set(FONT_RENDER_SOURCES
	"freetype2/src/base/ftsystem.c"
	"freetype2/src/base/ftinit.c"
	"freetype2/src/base/ftdebug.c"
	"freetype2/src/base/ftbase.c"
	"freetype2/src/truetype/truetype.c"
	"freetype2/src/sfnt/sfnt.c"
	"freetype2/src/smooth/smooth.c"
	"font_render.c"
	"font_cache.c"
)

include_directories(
	${ESP_SIMULATOR_INCLUDE_DIRECTORIES}
	"${CMAKE_SOURCE_DIR}/../components/font_render/include/"
	"${CMAKE_SOURCE_DIR}/../components/font_render/freetype2/include/"
	"$ENV{IDF_PATH}/components/log/include/"
	"$ENV{IDF_PATH}/components/esp_common/include/"
	"$ENV{IDF_PATH}/components/esp_event/include/"
	"$ENV{IDF_PATH}/components/"
	"${CMAKE_CURRENT_BINARY_DIR}/../../../config/"
)

add_definitions(-DFT2_BUILD_LIBRARY -Wno-unused-function -g3 -ggdb)

list(TRANSFORM FONT_RENDER_SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/../../components/font_render/)

add_library(font_render STATIC
	${FONT_RENDER_SOURCES}
)
