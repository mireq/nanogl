# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ram)

if (CONFIG_APP_BUILD_TYPE_ELF_RAM)
	set(project_elf ${CMAKE_PROJECT_NAME}.elf)
	set(project_bin ${CMAKE_PROJECT_NAME}.bin)
	add_custom_command(
		OUTPUT ${project_bin}
		COMMAND $ENV{IDF_PATH}/components/esptool_py/esptool/esptool.py --chip esp32 elf2image --flash_freq 40m --flash_mode dio --flash_size 4MB --spi-connection SPI ${project_elf}
		DEPENDS ${project_elf}
	)
	add_custom_target(_project_binary ALL DEPENDS ${project_bin})
	add_dependencies(_project_binary ${project_elf})
endif ()


if (CONFIG_BUILD_SIMULATOR)
	idf_build_get_property(config_dir CONFIG_DIR)
	set(sdkconfig_header ${config_dir}/sdkconfig.h)
	set(project_elf ${CMAKE_PROJECT_NAME}.elf)
	ExternalProject_Add(
		linux
		SOURCE_DIR "${CMAKE_SOURCE_DIR}/linux"
		INSTALL_COMMAND ""
		TEST_COMMAND ""
		BUILD_ALWAYS TRUE
		DEPENDS ${project_elf}
		CMAKE_ARGS -DCMAKE_ROOT_BUILD_DIR=${CMAKE_BINARY_DIR} -DESP_CONFIG_DIR=${config_dir}
	)
endif ()
