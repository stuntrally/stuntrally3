import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps
from conan.tools.files import save, copy


class StuntRally3(ConanFile):
    name = "StuntRally3"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "system_pkgs": [True, False],
    }
    default_options = {
        "system_pkgs": False,
        "bullet3/*:extras": True,
        "bullet3/*:network_support": True,
        "sdl/*:sdl2main": False,
    }

    def layout(self):
        self.folders.generators = os.path.join(self.folders.build, "generators")

    def requirements(self):
        self.requires("ogre3d-next/2024.09@anotherfoxguy/stable", force=True)
        self.requires("mygui-next/2024.01@anotherfoxguy/stable")

        if not self.options.system_pkgs:
            self.requires("boost/1.83.0")
            self.requires("bullet3/3.25@anotherfoxguy/patched") 
            self.requires("sdl/2.30.7")
            self.requires("ogg/1.3.5")
            self.requires("vorbis/1.3.7")
            self.requires("openal-soft/1.22.2@anotherfoxguy/patched")
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

        bindirs = []
        for dep in self.dependencies.values():
            bindirs += dep.cpp_info.bindirs

        bindirs_win = []
        for dir in bindirs:
            bindirs_win.append(os.path.join(dir, f"{self.settings.build_type}"))

        conan_data = 'set(CONAN_BIN_DIRS "%s;%s")\n' % (
            ";".join(bindirs).replace("\\", "/"),
            ";".join(bindirs_win).replace("\\", "/"),
        )

        save(
            self,
            os.path.join(self.build_folder, "cmake", "ConanBinDirs.cmake"),
            conan_data,
        )

        if self.settings.os == "Windows":
            libdir = os.path.join(self.source_folder, "bin", f"{self.settings.build_type}")
            for f in self.dependencies["ogre3d-next"].cpp_info.bindirs:
                copy(self, "*.dll", f, libdir, False)
