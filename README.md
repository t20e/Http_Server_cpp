# Http Server Built On C++ From Scratch

**Third-party software**:

1. Conan package manager
2. Google Test (GTest)
3. SQLite database.
4. [Bcrypt.cpp](https://github.com/hilch/Bcrypt.cpp) | Hashing passwords.
5. [jwt-cpp](https://github.com/Thalhammer/jwt-cpp/tree/master) | Handling User's session.
6. React: Frontend; solely for demonstration purposes.
7. Conan package manager

**Objectives**:

- Client can:
  1. Register & login
  2. Retrieve cat images.

- HTTP server is Multi-Threaded.


### How To Run Project:

1. Install all necccasty packages for backend and frontend
2. For VSC clangD extension to work properly bring make a symlink from the ./build/Release/compile_commands.json to the project's root. with command:
```bash
ln -s build/Release/compile_commands.json # Run this line if you ever delete the /build again!
# Restart clangD → VSC keyboard shortcut: Cmd + Shift + P -> clangd: Restart language server
```
1. Start cpp HTTP backend
2. Build the react app
3. serve the react build app
