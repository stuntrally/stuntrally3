import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps
from conan.tools.files import copy


class StuntRally3(ConanFile):
    name = "StuntRally3"
    settings = "os", "compiler", "build_type", "arch"
    default_options = {
        "bullet3/*:extras": "True",
        "bullet3/*:network_support": "True",
        "sdl/*:sdl2main": "False",
    }

    def layout(self):
        self.folders.generators = os.path.join(self.folders.build, "generators")

    def requirements(self):
        self.requires("boost/1.83.0")
        self.requires("ogre3d-next/2024.01@anotherfoxguy/stable", force=True)
        self.requires("bullet3/3.25") 
        self.requires("sdl/2.28.5")
        self.requires("mygui-next/2024.01@anotherfoxguy/stable")
        self.requires("ogg/1.3.5")
        self.requires("vorbis/1.3.7")
        self.requires("openal/1.22.2")
        self.requires("enet/1.3.17")
        self.requires("tinyxml2/9.0.0")
        self.requires("rapidjson/cci.20230929", force=True)

        self.requires("libalsa/1.2.10", override=True)
        self.requires("libpng/1.6.40", override=True)
        self.requires("libwebp/1.3.2", override=True)
        self.requires("zlib/1.3", override=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()
        if self.settings.os == "Windows" and self.settings.build_type == "Release":
            deps.configuration = "RelWithDebInfo"
            deps.generate()

        for dep in self.dependencies.values():
            for f in dep.cpp_info.bindirs:
                self.cp_data(f)
            for f in dep.cpp_info.libdirs:
                self.cp_data(f)

    def cp_data(self, src):
        bindir = os.path.join(self.build_folder, "bin")
        copy(self, "*.dll", src, bindir, False)
        copy(self, "*.so*", src, bindir, False)
