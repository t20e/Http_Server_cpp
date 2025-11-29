
# This CMake is paired with Conanfile.py

cmake_minimum_required(VERSION 3.23) # Minimum CMake version

project(HttpServer VERSION 0.1.0 LANGUAGES C CXX) # CXX stands for C++

# ---------------------- COMPILERS & FLAGS ----------------------
set(CMAKE_CXX_STANDARD 23) # -std=c+23
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Generates compile_commands.json for the clangD VSC extension.


# ---------------------- EXTERNAL CONAN LIBRARIES ----------------------
find_package(jwt-cpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(picojson REQUIRED) # Solely for the jwt-cpp library, I handle all json API calls myself!

# ---------------------- EXTERNAL LIBRARIES FROM ./libs----------------------
# Handle libraries located at ./libs, these are typically packages that aren't distributed by Conan, or maybe Header-only libraries that I copied and pasted into ./libs.

# Bcrypt.cpp 
file(GLOB BCRYPT_SRCS "./libs/Bcrypt.cpp/*.cpp")
add_library(bcrypt_lib STATIC ${BCRYPT_SRCS})
target_include_directories(bcrypt_lib PUBLIC "./libs/Bcrypt.cpp") # Bcrypt.cpp is the folder!


# ---------------------- CORE LIBRARY ----------------------
# Core is for all the files that I created, excluding external libraries

file(GLOB_RECURSE CORE_SRCS ./src/*.cpp) # Find all source files (.cpp) in the src directory
message(STATUS "Files found in ./src: ${CORE_SRCS}.")
list(FILTER CORE_SRCS EXCLUDE REGEX "src/main.cpp$") # Filter out my project's main.cpp so it doesn't get compiled into the library.

message(STATUS "Files found in Core ./Lib: ${CORE_SRCS}.")

add_library(project_core STATIC ${CORE_SRCS})
target_include_directories(project_core PUBLIC ./include) # Include all the files in the ./include directory

 # link sqlite, bcrypt, etc...
target_link_libraries(project_core PUBLIC 
    bcrypt_lib
    jwt-cpp::jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLite::SQLite3
    picojson::picojson 
)

if(MSVC)
    # Windows flags
    target_compile_options(project_core PRIVATE /W4) # /W4 is for warning level 4, similar to the -Wall for mac.
else() 
    # Mac/Linux flags
    target_compile_options(project_core PRIVATE -Wall -Wextra) # -Wextra enables additional useful warnings that are not included in -Wall.
endif()

# ---------------------- MAIN EXECUTABLE ----------------------
message(STATUS "Configuring Main Executable: HttpServer")
add_executable(HttpServer src/main.cpp)
target_link_libraries(HttpServer PRIVATE project_core)

 