if (NOT TARGET boost::boost)
    find_package(Boost REQUIRED COMPONENTS system thread filesystem)
    message(STATUS "Adding boost::boost target")
    add_library(boost::boost INTERFACE IMPORTED)
    set_target_properties(boost::boost PROPERTIES
            INTERFACE_LINK_LIBRARIES "${Boost_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
            )
endif ()

if (NOT TARGET Bullet::Bullet)
    message(STATUS "Adding Bullet::Bullet target")
    add_library(Bullet::Bullet INTERFACE IMPORTED)
    set_target_properties(Bullet::Bullet PROPERTIES
            INTERFACE_LINK_LIBRARIES "${BULLET_LIBRARIES};BulletWorldImporter;BulletXmlWorldImporter"
            INTERFACE_INCLUDE_DIRECTORIES "${BULLET_ROOT_DIR}/${BULLET_INCLUDE_DIR}"
            )
endif ()

if (NOT TARGET rapidjson)
    message(STATUS "Adding rapidjson target")
    add_library(rapidjson INTERFACE IMPORTED)
    set_target_properties(rapidjson PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${RAPIDJSON_INCLUDE_DIRS}"
            )
endif ()