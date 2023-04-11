from conan import ConanFile


class StuntRally3(ConanFile):
    name = "StuntRally3"
    settings = "os", "compiler", "build_type", "arch"
    generators = (
        "CMakeDeps",
        "CMakeToolchain",
    )
    default_options = {
        "bullet3/*:extras": "True",
        "bullet3/*:network_support": "True",
        "sdl/*:sdl2main": "False",
    }

    def requirements(self):
        self.requires("boost/1.81.0")
        self.requires("ogre3d-next/2023.04@anotherfoxguy/stable" , override=True)
        self.requires(
            "bullet3/3.25@anotherfoxguy/patched"
        )  # Needs a patched to build on windows
        self.requires("sdl/2.26.1")
        self.requires("mygui-next/2023.04@anotherfoxguy/stable")
        self.requires("ogg/1.3.5")
        self.requires("vorbis/1.3.7")
        self.requires("openal/1.22.2")
        self.requires("enet/1.3.17")
        self.requires("tinyxml2/9.0.0")

        self.requires("libpng/1.6.39", override=True)
        self.requires("libwebp/1.3.0", override=True)
        self.requires("zlib/1.2.13", override=True)
