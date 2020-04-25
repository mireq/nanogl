idf_component_register(
	SRCS
		"st7789.c"
		"st7789_ngl_driver.c"
	INCLUDE_DIRS
		"include"
	REQUIRES
		"nanogl"
)
target_compile_options(${COMPONENT_LIB} PRIVATE -O3)
