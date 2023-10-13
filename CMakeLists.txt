
# This assumes you used scripts to get and build latest Ogre-next from sources
# And your Ogre source code is above, in ../Ogre/ogre-next and ../Ogre/ogre-next-deps
# Tested on GNU/Linux, Debian 11.
# Windows most likely broken and it probably needs Ogre inside Dependencies subdir here, can be linked.
# see CMake/DependenciesConfig.cmake
# Conan is used to get SR deps, or not if using FORCE_SYSTEM_DEPENDENCIES.


cmake_minimum_required( VERSION 3.16 )
#  Include path for additional CMake library finding scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/" ${CMAKE_MODULE_PATH})

#  Add a sensible build type default and warning because empty means no optimization and no debug info.
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "Configuration types")

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.22")
    cmake_policy(SET CMP0127 OLD)
endif ()

include(CMakeDependentOption)
include(Macros)
find_package(PkgConfig REQUIRED)
include(FeatureSummary)


project(StuntRally3)


#  User Options  to disable building components  --------
option(BUILD_GAME             "Build the game binary."    ON)
option(BUILD_EDITOR           "Build the track editor."   ON)
option(BUILD_MASTER_SERVER    "Build the game list master server."  OFF)
option(BUILD_TRANSL_TOOL      "Build the tool for translation updating, it creates .pot"  ON)

#  Linux prefers lower-case exe names
if (WIN32 OR APPLE)
    set(GAME_EXE StuntRally3)
    set(EDITOR_EXE SR-Editor3)
    set(SERVER_EXE SR-MasterServer)
    set(TRANSL_EXE SR-Translator)
else ()
    set(GAME_EXE stuntrally3)
    set(EDITOR_EXE sr-editor3)
    set(SERVER_EXE sr-masterserver)
    set(TRANSL_EXE sr-translator)
endif ()


#  Avoid source tree pollution
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

#  exe path  ----
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")

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
set(COMMON_SRC_DIRS
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
set(GAME_SRC_DIRS ${COMMON_SRC_DIRS})
list(APPEND GAME_SRC_DIRS ./src/game)
#list( APPEND GAME_SRC_DIRS ./src/sound )
#list( APPEND GAME_SRC_DIRS ./src/vdrift )
list( APPEND GAME_SRC_DIRS ./src/oics )  # libs
list( APPEND GAME_SRC_DIRS ./src/network )


#  editor
set(EDITOR_SRC_DIRS ${COMMON_SRC_DIRS})
list(APPEND EDITOR_SRC_DIRS ./src/editor)


#  exe list  game and editor
if (BUILD_GAME)
    list(APPEND EXE_LIST ${GAME_EXE})
endif ()
if (BUILD_EDITOR)
    list(APPEND EXE_LIST ${EDITOR_EXE})
endif ()

##  Executables
##------------------------------------------------------------------------------------------------------
foreach (EXE ${EXE_LIST})

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
            SDL2::SDL2
            Threads::Threads
            tinyxml::tinyxml
            tinyxml2::tinyxml2
            Vorbis::vorbis
            Vorbis::vorbisfile
    )

endforeach ()

message(STATUS "---------- SR end")

feature_summary(WHAT ALL)
