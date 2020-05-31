# Convert file to c source
#
# Designed to be run as a script with "cmake -P"

if(NOT DATA_FILE)
	message(FATAL_ERROR "DATA_FILE for converting must be specified")
endif()

if(NOT SOURCE_FILE)
	message(FATAL_ERROR "SOURCE_FILE destination must be specified")
endif()

file(READ "${DATA_FILE}" data HEX)

string(LENGTH "${data}" data_len)
math(EXPR data_len "${data_len} / 2")  # 2 hex bytes per byte

if(FILE_TYPE STREQUAL "TEXT")
	set(data "${data}00")  # null-byte termination
endif()

## Convert string of raw hex bytes to lines of hex
string(REGEX REPLACE "................................" "\t\\0\n" data "${data}")  # 16 bytes per line
string(REGEX REPLACE "[^\n]+$" "\t\\0\n" data "${data}")                           # last line
string(REGEX REPLACE "[0-9a-f][0-9a-f]" "0x\\0, " data "${data}")                  # hex formatted C bytes
string(REGEX REPLACE ", \n" ",\n" data "${data}")                                  # trim the last whitespace


## Come up with C-friendly variable name based on source file
# unless VARIABLE_BASENAME is set
if(NOT VARIABLE_BASENAME)
	get_filename_component(source_filename "${DATA_FILE}" NAME)
	string(MAKE_C_IDENTIFIER "${source_filename}" varname)
else()
	string(MAKE_C_IDENTIFIER "${VARIABLE_BASENAME}" varname)
endif()

function(append str)
	file(APPEND "${SOURCE_FILE}" "${str}")
endfunction()

function(append_line str)
	append("${str}\n")
endfunction()

file(WRITE "${SOURCE_FILE}" "/*\n")
append_line(" * Data converted from ${DATA_FILE}")
if(FILE_TYPE STREQUAL "TEXT")
	append_line(" * (null byte appended)")
endif()
append_line(" */\n")
append_line("#include <stddef.h>\n")

append_line("extern const char *_binary_${varname}_start;")
append_line("extern const char *_binary_${varname}_end;")
append_line("extern size_t ${varname}_length;\n")

append_line("static const char _binary_${varname}_data[] = {")
append("${data}")
append_line("};")
append_line("const char *_binary_${varname}_start = (const char *)_binary_${varname}_data;")
append_line("const char *_binary_${varname}_end = (const char *)_binary_${varname}_data + ${data_len};")
append_line("size_t ${varname}_length = ${data_len};")
