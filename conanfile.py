from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy, save, load
import os


class SerialConan(ConanFile):
    name = "serial"
    version = "1.0.0"
    license = "MIT"
    description = "Cross-platform serial port library (Gastronomous fork)"
    url = "https://github.com/Gastronomous-Technologies/serial"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "conan_CMakeLists.txt", "src/*", "include/*"

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmakelists = load(self, os.path.join(self.source_folder, "conan_CMakeLists.txt"))
        save(self, os.path.join(self.source_folder, "CMakeLists.txt"), cmakelists)
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "*.h", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"))
        copy(self, "*.a", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, "*.so", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["serial"]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["rt", "pthread"]
