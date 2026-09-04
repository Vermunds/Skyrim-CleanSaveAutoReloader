project(
	"${PROJECT_NAME}"
	VERSION "${PROJECT_VERSION}"
	LANGUAGES CXX
)

set(SOURCES
	"main.cpp"
	"PCH.h"
 )

source_group(
	TREE "${CMAKE_CURRENT_SOURCE_DIR}"
	FILES ${SOURCES}
)

set(VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Plugin.h")
configure_file(
	"${CSAR_ROOT_DIR}/cmake/Plugin.h.in"
	"${VERSION_HEADER}"
	@ONLY
)

source_group("src" FILES "${VERSION_HEADER}")

configure_file(
	"${CSAR_ROOT_DIR}/cmake/version.rc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	@ONLY
)

# Create DLL
add_library(${PROJECT_NAME}
	SHARED
	${SOURCES}
	"${VERSION_HEADER}"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	"${CMAKE_SOURCE_DIR}/.clang-format"
	"${CMAKE_SOURCE_DIR}/.editorconfig"
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
		CommonLibSSE::CommonLibSSE
		spdlog::spdlog
)

target_link_libraries(
	"${PROJECT_NAME}"
	PUBLIC
		spdlog::spdlog
		Version.lib
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
