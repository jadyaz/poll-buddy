# Stage 1: Build stage using an official Ubuntu base image with build tools
FROM ubuntu:20.04 as builder

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    libboost-all-dev \
    pkg-config \
    libjsoncpp-dev \
 && rm -rf /var/lib/apt/lists/*

# Set the working directory in the container
WORKDIR /app

# Copy the project files into the container
COPY . .

# Run build commands
RUN mkdir -p build && cd build && cmake .. && make

# Stage 2: Create a new image for runtime
FROM ubuntu:20.04

WORKDIR /app

# Copy the compiled binary from the build stage
COPY --from=builder /app/build/VoteApp /app/

# Copy static asset directory and template directory from the build stage
COPY --from=builder /app/src/static /app/static
COPY --from=builder /app/src/templates /app/templates
COPY --from=builder /app/src/views/script.js /app/static/script.js

# Ensure VoteApp is executable (if necessary)
# RUN chmod +x /app/VoteApp

# Expose the port
EXPOSE 8080

CMD ["./VoteApp"]
