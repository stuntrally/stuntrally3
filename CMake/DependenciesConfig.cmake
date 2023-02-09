set(CMAKE_THREAD_PREFER_PTHREAD YES)

find_package(Threads REQUIRED)

find_package(Boost REQUIRED COMPONENTS system thread filesystem wave)
find_package(OGRE REQUIRED CONFIG COMPONENTS Bites Overlay Paging RTShaderSystem MeshLodGenerator Terrain)
find_package(BULLET REQUIRED COMPONENTS system thread filesystem wave)
find_package(SDL2 REQUIRED CONFIG)
find_package(MyGUI REQUIRED)
find_package(OGG REQUIRED)
find_package(Vorbis REQUIRED)
find_package(OpenAL REQUIRED CONFIG)
find_package(tinyxml REQUIRED)
find_package(tinyxml2 REQUIRED)
