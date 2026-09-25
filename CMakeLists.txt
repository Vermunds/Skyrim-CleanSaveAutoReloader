project(
	SkyrimAutoReloader 				 	 	 # Sets the name of the project, and stores it in the variable PROJECT_NAME.
	VERSION 1.1.0						 # Project version.
	LANGUAGES CXX						 # Selects which programming languages are needed to build the project.
)

set(CSAR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(FORMATTED_NAME "Clean Save Auto-Reloader")
set(LICENSE "GPL-3.0-or-later WITH Modding Exception")
set(AUTHOR "Vermunds")

add_subdirectory("src/SkyrimAutoReloaderHelper")
add_subdirectory("src/SkyrimAutoReloader")

# This project builds more than one target, tell the parent build about them.
set(MOD_TARGETS SkyrimAutoReloader SkyrimAutoReloaderHelper PARENT_SCOPE)
