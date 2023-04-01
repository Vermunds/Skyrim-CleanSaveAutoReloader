project(
	"${PROJECT_NAME}Helper"
	VERSION "${PROJECT_VERSION}"
	LANGUAGES CXX
)

# Include guard
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "in-source builds are not allowed!")
endif()

set(SOURCES
	"main.cpp"
	"PCH.h"	
    )

source_group(
	TREE "${CMAKE_CURRENT_SOURCE_DIR}"
	FILES ${SOURCES}
)

configure_file(
	"${CMAKE_SOURCE_DIR}/cmake/version.rc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	@ONLY
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_definitions(
		_UNICODE
	)

	if (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
		add_compile_options(
			/MP	# Build with Multiple Processes
		)
	endif ()
endif ()

# Create DLL
add_executable(${PROJECT_NAME}
	WIN32
	${SOURCES}
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	"${CMAKE_SOURCE_DIR}/.clang-format"
	"${CMAKE_SOURCE_DIR}/.editorconfig"
)

target_compile_features(
	${PROJECT_NAME}
	PRIVATE
		cxx_std_20
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Dependencies
find_package(spdlog REQUIRED CONFIG)
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		spdlog::spdlog
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_compile_options(
		"${PROJECT_NAME}"
		PRIVATE
			"/sdl"	# Enable Additional Security Checks
			"/utf-8"	# Set Source and Executable character sets to UTF-8
			"/Zi"	# Debug Information Format

			"/permissive-"	# Standards conformance
			"/Zc:preprocessor"	# Enable preprocessor conformance mode

			"/wd4200" # nonstandard extension used : zero-sized array in struct/union

			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		"${PROJECT_NAME}"
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

target_compile_definitions(
	"${PROJECT_NAME}"
	PUBLIC
		BOOST_STL_INTERFACES_DISABLE_CONCEPTS
		WINVER=0x0601	# windows 7, minimum supported version by skyrim special edition
		_WIN32_WINNT=0x0601
)

target_link_libraries(
	"${PROJECT_NAME}"
	PUBLIC
	spdlog::spdlog
	Version.lib
	comctl32.lib
)


target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		PCH.h
)

target_include_directories(
	"${PROJECT_NAME}"
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(
	FILES
		"$<TARGET_FILE:${PROJECT_NAME}>"
	DESTINATION "SKSE/Plugins"
	COMPONENT "main"
)

install(
	FILES
		"$<TARGET_PDB_FILE:${PROJECT_NAME}>"
	DESTINATION "/"
	COMPONENT "pdbs"
)

if("${COPY_OUTPUT}")
	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${SKYRIM_DATA_PATH}/SKSE/Plugins/${PROJECT_NAME}.exe"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${SKYRIM_DATA_PATH}/SKSE/Plugins/${PROJECT_NAME}.pdb"
		VERBATIM
	)
endif()

