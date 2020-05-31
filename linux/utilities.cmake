function(add_embedded_file target embed_file embed_type)
	get_filename_component(embed_file "${embed_file}" ABSOLUTE)
	get_filename_component(name "${embed_file}" NAME)
	set(embed_srcfile "${CMAKE_CURRENT_BINARY_DIR}/${name}_gen.c")

	add_custom_command(OUTPUT "${embed_srcfile}"
		COMMAND "${CMAKE_COMMAND}"
		-D "DATA_FILE=${embed_file}"
		-D "SOURCE_FILE=${embed_srcfile}"
		-D "FILE_TYPE=${embed_type}"
		-P "${CMAKE_CURRENT_SOURCE_DIR}/data_file_embed_c.cmake"
		MAIN_DEPENDENCY "${embed_file}"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/data_file_embed_c.cmake"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		VERBATIM
	)

	set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${embed_srcfile}")
	target_sources("${target}" PRIVATE "${embed_srcfile}")
endfunction()
