set(CMAKE_THREAD_PREFER_PTHREAD YES)

find_package(Threads REQUIRED)

find_package(Boost REQUIRED)
find_package(Bullet REQUIRED)
find_package(enet REQUIRED)
find_package(MyGUI REQUIRED)
find_package(Ogg REQUIRED)
find_package(OGRE REQUIRED)
find_package(OpenAL REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(SDL2 REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(Vorbis REQUIRED)

include(AddMissingTargets)