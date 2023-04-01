cmake_minimum_required(VERSION 3.22) 	 # Sets the minimum required version of cmake for a project.
set(CMAKE_SYSTEM_VERSION 10.0)           # The version of the operating system for which CMake is to build.

# Set up vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()
set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "target triplet" FORCE)

project(
	SkyrimAutoReloader 				 	 	 # Sets the name of the project, and stores it in the variable PROJECT_NAME.
	VERSION 1.0.0						 # Project version.
	LANGUAGES CXX						 # Selects which programming languages are needed to build the project.
)

# Include guard
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "in-source builds are not allowed!")
endif()

if (MSVC)
	# Adds preprocessor definitions to the compiler command line.
	add_compile_definitions(
		_UNICODE
	)

	if (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
		add_compile_options(
			/MP	# Build with Multiple Processes
		)
	endif ()
endif ()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)
set(SKSE_SUPPORT_XBYAK ON)

option(COPY_OUTPUT "Copy the output of build operations to the game directory" OFF)
set(SKYRIM_DATA_PATH "" CACHE PATH "Skyrim Data folder path")

add_subdirectory("external/CommonLibSSE")
add_subdirectory("src/SkyrimAutoReloaderHelper")
add_subdirectory("src/SkyrimAutoReloader")
