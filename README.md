# Http Server Built On C++ From Scratch

Built using the *POSIX standard*.

**External software**:

1. Conan package manager
2. Google Test (GTest)
3. SQLite database.
4. [Bcrypt.cpp](https://github.com/hilch/Bcrypt.cpp) | Hashing passwords.
5. [jwt-cpp](https://github.com/Thalhammer/jwt-cpp/tree/master) | Handling User's session.
6. React: Frontend; solely for demonstration purposes.
7. Conan package manager
8. Docker

**Objectives**:

- Client can:
  1. Register & login.
  2. Retrieve cat images.
  3. Use previous session (JWT token) to stay logged in.

- Server is Multi-Threaded.
- Implement a logger.
- Testing with GTest.
- Implement Docker, Conan, and CMake.

## How To Run Project

[Project configurations](.env)

### Run Using Docker (Runs On All Operating Systems)

``` bash
# Clone and `cd` into project's root.
# Run the two containers: First one for the backend; and second for the frontend.
docker compose up --build --attach server; 
# The --attach server is to only see the logs of the backend.
```

### Or Run With Just Conan And Cmake (Mac Only)

1. Install all necessary packages for backend and frontend
2. For VSC clangD extension to work properly bring make a symlink from the ./build/Release/compile_commands.json to the project's root. with command:

```bash
ln -s build/Release/compile_commands.json # Run this line if you ever delete the /build again!
# Then Restart clangD → VSC keyboard shortcut: Cmd + Shift + P -> clangd: Restart language server
```

1. Start cpp HTTP backend
2. Build the react app
3. serve the react build app

**How to run unit tests:**

```zsh
conan build .
ctest --test-dir build/Release --output-on-failure # or run the test directly `./build/Release/unit_tests`
```
