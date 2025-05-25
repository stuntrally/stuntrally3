
# This CMake is for CI/CD auto builds in github Actions tab.
# See in docs/Building.md and BuildingVS.md for building from sources.
# Conan is used to get SR deps, or not if using FORCE_SYSTEM_DEPENDENCIES.


cmake_minimum_required( VERSION 3.16 )
#  Include path for additional CMake library finding scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/" ${CMAKE_MODULE_PATH})

#  Add a sensible build type default and warning because empty means no optimization and no debug info.
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "Configuration types")

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.22")
    cmake_policy(SET CMP0127 OLD)
endif ()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

include(CMakeDependentOption)
include(Macros)
include(FeatureSummary)

if (NOT WIN32)
    find_package(PkgConfig)
endif ()

project( StuntRally3 )


#  User Options  to disable building components  --------
option(BUILD_GAME           "Build the game binary."    ON)
option(BUILD_EDITOR         "Build the track editor."   ON)
option(BUILD_GAME_SERVER    "Build the game list server app."  ON)
option(BUILD_TRANSL_TOOL    "Build the tool for translation updating, it creates .pot"  ON)


#  Linux prefers lower-case exe names
if (WIN32 OR APPLE)
    set(GAME_EXE StuntRally3)
    set(EDITOR_EXE SR-Editor3)
    set(SERVER_EXE SR-GameServer)
    set(TRANSL_EXE SR-Translator)
else ()
    set(GAME_EXE stuntrally3)
    set(EDITOR_EXE sr-editor3)
    set(SERVER_EXE sr-gameserver)
    set(TRANSL_EXE sr-translator)
endif ()


#  Avoid source tree pollution
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

#  exe path  ----
set( EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )

#  Set CXX compile flags
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    add_definitions(-DUNICODE -D_UNICODE)
endif ()


#  deps  ----

#  Check for dependencies
include(DependenciesConfig)

macro(add_recursive dir retVal)
    file(GLOB GLOB_RESULT ${dir}/*.h ${dir}/*.cpp ${dir}/*.c)
    list(APPEND ${retVal} ${GLOB_RESULT})
endmacro()

message(STATUS "---------- SR sources")

#  Compiler  ----
set( COMMON_SRC_DIRS
	#  not really sources
	data/gui
	data/materials
	data/particles

	#  libs
	./src/btOgre2
	./src/libs

	#  sources
	./src/common
	./src/common/data
	./src/common/MessageBox
	./src/road

	#  Ogre sources
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
	./src/sound
	./src/vdrift
)

#  game
set( GAME_SRC_DIRS ${COMMON_SRC_DIRS} )
list( APPEND GAME_SRC_DIRS ./src/game )
list( APPEND GAME_SRC_DIRS ./src/game/data )
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
endif ()
if (BUILD_EDITOR)
    list( APPEND EXE_LIST ${EDITOR_EXE} )
endif ()

##  Executables
##------------------------------------------------------------------------------------------------------
foreach( EXE ${EXE_LIST} )

    message(STATUS "Configuring: " ${EXE})

    #  sources  ----
    set(EXE_SOURCES "")

    if (${EXE} STREQUAL ${EDITOR_EXE})  # ed
        set(SRC_DIRS ${EDITOR_SRC_DIRS})
    else ()
        set(SRC_DIRS ${GAME_SRC_DIRS})
    endif ()

    foreach (subdir ${SRC_DIRS})
        #message( STATUS ${subdir} )
        add_recursive(${subdir} EXE_SOURCES)
    endforeach ()

    #  exe  ----
    add_executable(${EXE} WIN32 ${EXE_SOURCES})

    # Generate source groups for use in IDEs
    source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${EXE_SOURCES})

    foreach (subdir ${SRC_DIRS})
        #message( STATUS ${subdir} )
        target_include_directories(${EXE} PRIVATE ${subdir})
    endforeach ()
    #  pch.h
    if (${EXE} STREQUAL ${EDITOR_EXE})  # ed
        set_target_properties(${EXE} PROPERTIES COMPILE_FLAGS "-DSR_EDITOR")
        target_precompile_headers(${EXE} PRIVATE src/editor/pch.h)
    else ()
        target_precompile_headers(${EXE} PRIVATE src/game/pch.h)
    endif ()

    target_link_libraries(
            ${EXE} 
            PRIVATE
            boost::boost
            Bullet::Bullet
            enet::enet
            MyGUI::MyGUI
            Ogg::ogg
            OGRE::OGRE
            OpenAL::OpenAL
            rapidjson
            SDL2::SDL2
            Threads::Threads
            tinyxml2::tinyxml2
            Vorbis::vorbis
            Vorbis::vorbisfile
    )

    if (WIN32)
        install(TARGETS ${EXE} DESTINATION .)

        # add a post-build command to copy DLLs beside the executable
        add_custom_command(
            TARGET ${EXE}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -DEXECUTABLE="$<TARGET_FILE:${EXE}>"
                -DBIN_DIR="${CMAKE_BINARY_DIR}"
                -DLIB_DIR="$<TARGET_FILE_DIR:${EXE}>"
                -P "${CMAKE_SOURCE_DIR}/CMake/FixupBundle.cmake"
        )
    endif ()
endforeach ()

##  Translations tool
##------------------------------------------------------------------------------------------------------
if (BUILD_TRANSL_TOOL)
    message(STATUS "Configuring: " ${TRANSL_EXE})

    add_executable(${TRANSL_EXE} WIN32 ./src/transl/main.cpp)
    target_include_directories(${TRANSL_EXE} PRIVATE ./src/transl)

    target_precompile_headers(${TRANSL_EXE} PRIVATE src/transl/pch.h)
endif()

message(STATUS "---------- SR end")


#  Install targets
# -----------------------

if (WIN32)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION . PATTERN ".git/*" EXCLUDE)
    install(DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/ DESTINATION . FILES_MATCHING PATTERN "*.dll" )
    install(FILES ${CMAKE_SOURCE_DIR}/dist/plugins_Windows.cfg DESTINATION . RENAME plugins.cfg)
    install(FILES ${CMAKE_SOURCE_DIR}/dist/resources2.cfg DESTINATION .)
endif ()


#  CPack
# -----------------------
set(CPACK_PACKAGE_NAME "Stunt Rally")
set(CPACK_PACKAGE_FILE_NAME "stunt-rally-${CMAKE_PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "Stunt Rally is a 3D racing game with own Track Editor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Stunt Rally is a 3D racing game with own Track Editor")
set(CPACK_PACKAGE_VENDOR "Crystal Hammer")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_EXECUTABLES "StuntRally3" "Stunt Rally 3")

if (WIN32)
    set(CPACK_GENERATOR INNOSETUP)
    set(CPACK_ADD_REMOVE TRUE)
    set(CPACK_CREATE_DESKTOP_LINKS "StuntRally3" "Stunt Rally 3")

    set(CPACK_INNOSETUP_ALLOW_CUSTOM_DIRECTORY ON)
    set(CPACK_INNOSETUP_USE_MODERN_WIZARD ON)
    # set(CPACK_INNOSETUP_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sr3.ico")
    set(CPACK_INNOSETUP_SETUP_AppId "{{67B750C8-691F-4AAB-A79C-7557314B24A4}")

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
    # set(CPACK_INNOSETUP_SETUP_DiskSpanning True)
else ()
    set(CPACK_GENERATOR 7Z)
endif ()

include(CPack)


feature_summary(WHAT ALL)
