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
	#"font_render.c"
)

include_directories(
	"${CMAKE_SOURCE_DIR}/../components/font_render/include/"
	"${CMAKE_SOURCE_DIR}/../components/font_render/freetype2/include/"
	"${ESP_CONFIG_DIR}"
)

add_definitions(-DFT2_BUILD_LIBRARY -Wno-unused-function)

list(TRANSFORM FONT_RENDER_SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/../../components/font_render/)

add_library(font_render STATIC
	${FONT_RENDER_SOURCES}
)
