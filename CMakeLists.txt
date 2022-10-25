#
# This assumes you used scripts to get and build latest Ogre-next from sources
# And your Ogre source code is above, in ../Ogre/ogre-next and ../Ogre/ogre-next-deps
# Tested on Linux.  Windows most likely broken..
#

cmake_minimum_required( VERSION 3.0 )
project( TerrainDemo )


set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )

include( CMake/Bootstrap.cmake )
include( CMake/Dependencies/OGRE.cmake )


#  root dir
get_filename_component(DIR_ONE_ABOVE ../ ABSOLUTE)
message(STATUS ${DIR_ONE_ABOVE})

set(OGRE_NEXT_DIR ${DIR_ONE_ABOVE}/Ogre/ogre-next)

## Fix 1
set(OGRE_SOURCE ${OGRE_NEXT_DIR}/)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(OGRE_BINARIES ${OGRE_NEXT_DIR}/build/Debug)
else()
	set(OGRE_BINARIES ${OGRE_NEXT_DIR}/build/Release)
endif()


#This scripts will add all the cpp and h files from src and include folders
setupOgre( OGRE_SOURCE, OGRE_BINARIES, OGRE_LIBRARIES, FALSE, FALSE )

## Fix 2 sdl
include_directories("/usr/include/SDL2")
include_directories(${DIR_ONE_ABOVE}/Ogre/ogre-next-deps/build/ogredeps/include)  #for rapidjson


# Setup our application
set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )
if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	if( NOT PLATFORM_X64 )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	endif()
	add_definitions( -DUNICODE -D_UNICODE )
endif()


macro( add_recursive dir retVal )
	file( GLOB_RECURSE ${retVal} ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
endmacro()


#  compiler
include_directories(
	src
	src/game
	src/OgreCommon
	src/OgreCommon/System
	src/OgreCommon/System/Desktop
	src/OgreCommon/Threading
	src/OgreCommon/Utils
	src/Terra
	src/Terra/Hlms
	src/Terra/Hlms/PbsListener
)

add_recursive( ./src GAME_SOURCES )


add_executable( ${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${GAME_SOURCES} )


target_compile_options( ${PROJECT_NAME} PRIVATE -Wswitch)

#  linker
target_link_libraries( ${PROJECT_NAME} ${OGRE_LIBRARIES} )
## Fix 3 sdl
target_link_libraries( ${PROJECT_NAME} "-lSDL2" )
