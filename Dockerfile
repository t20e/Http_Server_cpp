# Docker for the backend, the frontend has its own Dockerfile
# Using Multi-Stage Build:

# ---------------------- Stage 1: Build The Application----------------------
# FROM ubuntu:24.04 AS builder
# debian:stable-slim is smaller than ubuntu:24.04
FROM debian:stable-slim AS builder

#Install necessary system tools and compilers
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    python3-pip \
    pkg-config \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install Conan
RUN pip3 install conan --break-system-packages
RUN conan profile detect --force

# Copy dependencies from project to the container
WORKDIR /app 
COPY conanfile.py .
COPY CMakeLists.txt .

# Install dependencies
RUN conan install . --build=missing -s compiler.cppstd=23

# Copy the rest of the source code from project to the container
COPY ./src ./src
COPY ./include ./include
COPY ./libs ./libs
COPY ./tests ./tests
COPY ./server_images ./server_images
# Or to copy fill source into the container just use below line:
# COPY . . # but copying what u only need it better!

# Build the project
WORKDIR /app
RUN conan build .  -s compiler.cppstd=23


# ---------------------- Stage 2: Run The Application----------------------
# FROM ubuntu:24.04 AS runner
FROM debian:stable-slim AS runner 

# Create a non-root user for security
RUN useradd -m httpServerUser
USER httpServerUser
WORKDIR /app

# Copy the executable from project to the container
COPY --from=builder --chown=httpServerUser:httpServerUser /app/build/Release/HttpServer .
COPY --chown=httpServerUser:httpServerUser ./server_images ./server_images
# Copy any other runtime assets if needed (like HTML temples, etc..)
# 🚨 SECURITY NOTE: In a real-world project you would never bake the .env or any other private settings into the image, because anyone u share the image with, will get the secrets.
COPY --chown=httpServerUser:httpServerUser .env .

# Expose port 8080 for the HTTP backend server
EXPOSE 8080

# Run the executable
CMD ["./HttpServer"]

# Build the image command:
# docker build -t cpp_http_server .

# Note use compose.yaml to run this project's image, not below command!
#       Command to run this container
#           docker run -p 8080:8080 cpp_http_server
