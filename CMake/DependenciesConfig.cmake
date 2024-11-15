include(DependenciesFunctions)

set(CMAKE_THREAD_PREFER_PTHREAD YES)
find_package(Threads REQUIRED)

# Some pkg-config files are broken, that is why they are commented out

#  fails
#add_external_lib(
#        Boost
#        boost/1.81.0
#        REQUIRED
#        FIND_PACKAGE_OPTIONS COMPONENTS system thread
#)

#add_external_lib(
#        OGRE
#        ogre3d/13.4.4@anotherfoxguy/stable
#        REQUIRED
#        CONAN_PKG_NAME OGRE
#        CONAN_OPTIONS ogre3d:nodeless_positioning=True ogre3d:resourcemanager_strict=off
#        #  uncomment below for Ogre 1.9
#        #PKG_CONFIG "OGRE, OGRE-Terrain, OGRE-Paging, OGRE-Overlay"        
#        FIND_PACKAGE_OPTIONS CONFIG COMPONENTS Bites Overlay Paging RTShaderSystem MeshLodGenerator Terrain
#)

add_external_lib(
        BULLET
        bullet3/3.24
        REQUIRED
        FIND_PACKAGE
        CONAN_OPTIONS bullet3:extras=True bullet3:network_support=True
        INTERFACE_NAME Bullet::Bullet
)

#add_external_lib(
#        SDL2
#        sdl/2.0.20
#        REQUIRED
#        PKG_CONFIG "sdl2 >= 2.0"
#        FIND_PACKAGE_OPTIONS CONFIG
#        CONAN_OPTIONS sdl:sdl2main=False
#)

#add_external_lib(
#        MyGUI
#        mygui/3.4.1@anotherfoxguy/stable
#        REQUIRED
#        # PKG_CONFIG "MYGUI = 3.4.0"
#        FIND_PACKAGE
#)

add_external_lib(
        OGG
        ogg/1.3.5
        REQUIRED
        PKG_CONFIG "ogg >= 1.2"
        FIND_PACKAGE
)

add_external_lib(
        VorbisFile
        vorbis/1.3.7
        REQUIRED
        PKG_CONFIG "vorbis >= 1.2, vorbisfile >= 1.2"
        FIND_PACKAGE
)

#  todo: fix  Cannot load recipe.
#add_external_lib(
#        OpenAL
#        openal/1.22.2
#        REQUIRED
#        PKG_CONFIG "openal >= 1.18"
#        FIND_PACKAGE_OPTIONS CONFIG
#)

#add_external_lib(
#        ENet
#        enet/1.3.17
#        REQUIRED
#        PKG_CONFIG "libenet >= 1.2"
#        FIND_PACKAGE
#)

add_external_lib(
        tinyxml2
        tinyxml2/9.0.0
        REQUIRED
        PKG_CONFIG "tinyxml2 >= 6"
        FIND_PACKAGE
)


set(LIBS
        #Boost::Boost  #  fails
        #Threads::Threads
        #OGRE::OGRE
        #- Bullet::Bullet  // fixme?
        #SDL2::SDL2
        #MyGUI::MyGUI
        VorbisFile::VorbisFile
        OGG::OGG
        #OpenAL::OpenAL  #  fails
        #ENet::ENet  #  fails
        tinyxml2::tinyxml2
)

#set(SERVER_LIBS Boost::Boost ENet::ENet)


find_package( ENet REQUIRED )

find_package( Boost COMPONENTS system thread REQUIRED )
