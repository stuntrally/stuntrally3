
# This assumes you used scripts to get and build latest Ogre-next from sources
# And your Ogre source code is above, in ../Ogre/ogre-next and ../Ogre/ogre-next-deps
# Tested on GNU/Linux, Debian 11.
# Windows most likely broken and it probably needs Ogre inside Dependencies subdir here, can be linked.
# see CMake/DependenciesConfig.cmake
# Conan is used to get SR deps, or not if using FORCE_SYSTEM_DEPENDENCIES.


cmake_minimum_required( VERSION 3.16 )

message(STATUS "Build DIR: ${CMAKE_BINARY_DIR}")


#  Include path for additional CMake library finding scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/" ${CMAKE_MODULE_PATH})

#  Add a sensible build type default and warning because empty means no optimization and no debug info.
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "Configuration types")

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.22")
    cmake_policy(SET CMP0127 OLD)
endif()

include(CMakeDependentOption)
include(Macros)
include(FeatureSummary)


project( StuntRally3 )


#  User Options  to disable building components  --------
option(BUILD_GAME             "Build the game binary."    ON)
option(BUILD_EDITOR           "Build the track editor."   ON)
option(BUILD_MASTER_SERVER    "Build the game list master server."  OFF)
option(BUILD_TRANSL_TOOL      "Build the tool for translation updating, it creates .pot"  OFF)


#  Linux prefers lower-case exe names
if (WIN32 OR APPLE)
    set(GAME_EXE StuntRally3)
    set(EDITOR_EXE SR-Editor3)
    set(SERVER_EXE SR-MasterServer)
    set(TRANSL_EXE SR-Translator)
else()
    set(GAME_EXE stuntrally3)
    set(EDITOR_EXE sr-editor3)
    set(SERVER_EXE sr-masterserver)
    set(TRANSL_EXE sr-translator)
endif()

set(_PREFIX "SR_")

#  Test if conan is installed
find_program(CONAN_CMD_TEST conan NO_CACHE)
cmake_dependent_option(USE_PACKAGE_MANAGER "Use conan" ON "NOT CONAN_CMD_TEST STREQUAL CONAN_CMD_TEST-NOTFOUND" OFF)
message(STATUS "Using conan: ${USE_PACKAGE_MANAGER}")

#  Allow user to globally set the library preference for external libraries
cmd_option(${_PREFIX}LIB_PREFERENCE
        "Library preference [SYSTEM (if available), CONAN]"
        "CONAN"
        STRINGS "SYSTEM" "CONAN"
        )

#  Special mode, that will force dependencies to the packages provided by system unless they were set to conan explicitly.
option(${_PREFIX}FORCE_SYSTEM_DEPENDENCIES "Force the use of system packages")


#  Avoid source tree pollution
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)


#  exe path  ----
set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )

#  Set CXX compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-comment")

if (CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	if( NOT PLATFORM_X64 )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	endif()
	add_definitions( -DUNICODE -D_UNICODE )
endif()


#  deps  ----
include( CMake/Bootstrap.cmake )
include( CMake/Dependencies/OGRE.cmake )

#  Check for dependencies
include(DependenciesConfig)

message( STATUS "---------- resolve_conan_dependencies" )
resolve_conan_dependencies()


#  root dir
message( STATUS "---------- root dir above" )
get_filename_component( DIR_ONE_ABOVE ../ ABSOLUTE )
message( STATUS ${DIR_ONE_ABOVE} )

set( OGRE_NEXT_DIR ${DIR_ONE_ABOVE}/Ogre/ogre-next )

#  Fix 1
set( OGRE_SOURCE ${OGRE_NEXT_DIR}/ )

if ( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	set( OGRE_BINARIES ${OGRE_NEXT_DIR}/build/Debug )
else()
	set( OGRE_BINARIES ${OGRE_NEXT_DIR}/build/Release )
endif()


#  This scripts will add all the cpp and h files from src and include folders
message( STATUS "---------- setup Ogre" )

setupOgre( OGRE_SOURCE, OGRE_BINARIES, OGRE_LIBRARIES, FALSE, FALSE )

#  Fix 2  SDL
include_directories( "/usr/include/SDL2" )
include_directories( ${DIR_ONE_ABOVE}/Ogre/ogre-next-deps/build/ogredeps/include )  #for rapidjson

#  MyGui
include_directories( ${DIR_ONE_ABOVE}/mygui-next/MyGUIEngine/include )
include_directories( ${DIR_ONE_ABOVE}/mygui-next/Platforms/Ogre2/Ogre2Platform/include )


macro( add_recursive dir retVal )
	file( GLOB GLOB_RESULT ${dir}/*.h ${dir}/*.cpp ${dir}/*.c )
	list( APPEND ${retVal} ${GLOB_RESULT})
endmacro()

message( STATUS "---------- SR sources" )

#  Compiler  ----
set( COMMON_SRC_DIRS
	#  not really sources
	Media/gui
	Media/materials
	Media/particles

	#  libs
	./src/btOgre2
	
	#  sources
	./src/common
	./src/common/data
	./src/common/MessageBox
	./src/road
	./src/OgreCommon
	./src/OgreCommon/System
	./src/OgreCommon/System/Android
	./src/OgreCommon/System/Desktop
	./src/OgreCommon/Threading
	./src/OgreCommon/Utils
	./src/Terra
	./src/Terra/Hlms
	./src/Terra/Hlms/PbsListener
	#  both game and ed  meh-
	#./src/vdriftCommon
	./src/sound
	./src/vdrift
)

#  game
set( GAME_SRC_DIRS ${COMMON_SRC_DIRS} )
list( APPEND GAME_SRC_DIRS ./src/game )
#list( APPEND GAME_SRC_DIRS ./src/sound )
#list( APPEND GAME_SRC_DIRS ./src/vdrift )
list( APPEND GAME_SRC_DIRS ./src/oics )  # libs
list( APPEND GAME_SRC_DIRS ./src/network )


#  editor
set( EDITOR_SRC_DIRS ${COMMON_SRC_DIRS} )
list( APPEND EDITOR_SRC_DIRS ./src/editor )


#  exe list  game and editor
if (BUILD_GAME)
	list( APPEND EXE_LIST ${GAME_EXE} )
endif()
if (BUILD_EDITOR)
	list( APPEND EXE_LIST ${EDITOR_EXE} )
endif()



#  fix 4  OpenAL  since conan failed  - common, meh
find_package( OpenAL REQUIRED )
include_directories( ${OPENAL_INCLUDE_DIR} )

find_package( Threads )

find_package( Boost COMPONENTS system thread REQUIRED )


##  Executables
##------------------------------------------------------------------------------------------------------
foreach( EXE ${EXE_LIST} )

	message( STATUS "Configuring: " ${EXE} )

	#  sources  ----
	set( EXE_SOURCES "")

	if ( ${EXE} STREQUAL ${EDITOR_EXE} )  # ed
		set( SRC_DIRS ${EDITOR_SRC_DIRS} )
	else()
		set( SRC_DIRS ${GAME_SRC_DIRS} )
	endif()

	foreach( subdir ${SRC_DIRS} )
		#message( STATUS ${subdir} )
		add_recursive( ${subdir} EXE_SOURCES )
	endforeach()

	#  exe  ----
	add_executable( ${EXE} WIN32 ${EXE_SOURCES} )


	#  add includes  ----
	foreach( subdir ${SRC_DIRS} )
		#message( STATUS ${subdir} )
		target_include_directories( ${EXE} PRIVATE ${subdir} )
	endforeach()

	#  fix 5  ENet  since conan fails
	# find_package( ENet REQUIRED )  # inside Dep..cmake
	if ( ${EXE} STREQUAL ${GAME_EXE} )  # game, once
		target_include_directories( ${EXE} PRIVATE ${ENet_INCLUDE_DIRS} )
		target_include_directories( ${EXE} PRIVATE ${Boost_INCLUDE_DIRS} )
	endif()


	#  pch.h
	if ( ${EXE} STREQUAL ${EDITOR_EXE} )  # ed
	    set_target_properties( ${EXE} PROPERTIES COMPILE_FLAGS "-DSR_EDITOR" )
		target_precompile_headers( ${EXE} PRIVATE src/editor/pch.h )
	else()
		target_precompile_headers( ${EXE} PRIVATE src/game/pch.h )
	endif()

	#  c++
	target_compile_options(  ${EXE} PRIVATE -Wswitch )
	target_compile_features( ${EXE} PRIVATE cxx_std_17 )


	#  add linker  ----
	#  threads
	target_link_libraries( ${EXE} ${CMAKE_THREAD_LIBS_INIT} )


	#  OpenAL
	target_link_libraries( ${EXE} "${OPENAL_LIBRARY}" )


	#  Ogre  ----
	target_link_libraries( ${EXE} ${OGRE_LIBRARIES} )

	#  SDL
	target_link_libraries( ${EXE} "-lSDL2" )


	#  MyGui  ----   # todo: windows ..
	if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    	set(SUFFIX_D "_d")
	else()
		set(SUFFIX_D "")
	endif()

	target_link_libraries( ${EXE} ${DIR_ONE_ABOVE}/mygui-next/build/${CMAKE_BUILD_TYPE}/lib/libMyGUI.Ogre2Platform${SUFFIX_D}.a )

	if ( ${EXE} STREQUAL ${GAME_EXE} )  # game, once
		add_library( libMyGUIEngine SHARED IMPORTED )
	endif()
	set_target_properties( libMyGUIEngine PROPERTIES IMPORTED_LOCATION ${DIR_ONE_ABOVE}/mygui-next/build/${CMAKE_BUILD_TYPE}/lib/libMyGUIEngine${SUFFIX_D}.so )
	target_link_libraries( ${EXE} libMyGUIEngine )


	#  ENet, boost  for networking  ----
	if ( ${EXE} STREQUAL ${GAME_EXE} )  # game, once
		target_link_libraries( ${EXE} ${Boost_LIBRARIES} )
		target_link_libraries( ${EXE} ${ENet_LIBRARIES} )
	endif()

	#  rest
	#message( STATUS "Libs  " ${EXE} "  " ${LIBS} )
	target_link_libraries( ${EXE} ${LIBS} )

endforeach()

message( STATUS "---------- SR end" )
