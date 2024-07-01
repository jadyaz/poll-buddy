# Use an official Ubuntu base image
FROM ubuntu:20.04

# Avoid being prompted during the build process
ARG DEBIAN_FRONTEND=noninteractive

# Install dependencies and compilers
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    libboost-all-dev \
    pkg-config \
    libjsoncpp-dev

# Clean up the apt cache to reduce image size
RUN rm -rf /var/lib/apt/lists/*

# Set the working directory in the container to /app
WORKDIR /app

# Copy the current directory contents into the container at /app
COPY . .

# Create and move to the build directory to keep the build out of source
RUN mkdir -p build
WORKDIR /app/build

# Run cmake and make in the build directory
RUN cmake .. && make

# Expose the port the app runs on (8080 in this case)
EXPOSE 8080

# Run the built application
CMD ["./VoteApp"] # Make sure this path points to the actual built executable