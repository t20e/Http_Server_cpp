import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMake, cmake_layout


class CompressorRecipe(ConanFile):

    # settings defines the project-wide variables, like the compiler, its version, or the OS
    settings = "os", "compiler", "build_type", "arch"

    # Specifies which Conan generators will be run when we call the `conan install` command.
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        """
        External libraries
        """

        # Example: dependency for a specific OS
        # if self.settings.os == "Windows":
        #     self.requires("base64/0.4.0")

        self.requires("jwt-cpp/0.7.1")
        self.requires("openssl/3.6.0")  # Required for jwt-cpp
        self.requires("sqlite3/3.51.0")
        self.requires(
            "picojson/1.3.0"
        )  # Solely for the jwt-cpp library, I handle all json API calls myself!
        self.requires("gtest/1.17.0")

    def build_requirements(self):
        self.tool_requires("cmake/3.27.9")

        # Example: Use the system's CMake for Windows
        # if self.settings.os != "Windows":
        #     self.tool_requires("cmake/3.27.9")

    def build(self):
        """
        Allows us to use the `conan build .` command to build, instead of having to type out all the commands, e.g., `conan install + cmake<configure> ...`. Conan automatically handles the environment setup, directory changes, and passing the correct arguments, and portable across operation systems.

        Note: The conan build command does not intend to replace or change the typical developer flow using CMake and other build tools, and using their IDEs. It is just a convenient shortcut for cases in which we want to locally build a project easily without having to type several commands.
        """
        cmake = CMake(
            self
        )  # Creates the CMake helper object referencing the recipe's self context.

        cmake.configure()  # Performs the commands `cd build/Release` `source ./generators/conanbuild.sh ` cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release`

        cmake.build()  # Performs the command `cmake --build`

        # Run the executable with command `./build/etc.../{executable_that_is_defined_in_CMakeLists.txt}`

    def layout(self):
        """
        Define a layout for where to place the build.
        """
        cmake_layout(
            self
        )  # Lets Conan take care of this layout, sets up build/Release, build/generators, etc...

    def validate(self):
        """
        Validate Conan configurations
        """

        # Make sure that the default config at `~/.conan2/profiles/default` is using C++23
        min_cppstd = 23
        if int(self.settings.get_safe("compiler.cppstd")) < min_cppstd:
            raise ConanInvalidConfiguration(
                f"This recipe requires C++{min_cppstd} or newer, please updated @ `~/.conan2/profiles/default` → line → compiler.cppstd"
            )
