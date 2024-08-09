# gRPC Study during GMD SOFT Internship Period

## OS
- Linux , MacOS

## Libraries
- OpenCV4
- YOLOv4

## Version
- v1.0
  - https://github.com/YooJuno/study-gRPC/tree/v1.0_File-Download-Sync
- v1.1
  - https://github.com/YooJuno/study-gRPC/tree/v1.1_File-Download-Sync-Stream
- v2.0
  - https://github.com/YooJuno/study-gRPC/tree/v2.0_Mat-Sync
- v2.1
  - https://github.com/YooJuno/study-gRPC/tree/v2.1_Mat-Callback
- v2.2
  - https://github.com/YooJuno/study-gRPC/tree/v2.2_Mat-Async

## Installation gRPC

### Setup

```bash
export MY_INSTALL_DIR=$HOME/.local
mkdir -p $MY_INSTALL_DIR
export PATH="$MY_INSTALL_DIR/bin:$PATH"
```

### **Install cmake**

You need version 3.13 or later of `cmake`. Install it by following these instructions:

- Linux
    
    ```bash
    sudo apt install -y cmake
    ```
    
- macOS:
    
    ```bash
    brew install cmake
    ```
    

Check the version of `cmake`:

```bash
$ cmake --version
```


Under Linux, the version of the system-wide `cmake` can often be too old. You can install a more recent version into your local installation directory as follows:

```bash
wget -q -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.19.6/cmake-3.19.6-Linux-x86_64.sh
sh cmake-linux.sh -- --skip-license --prefix=$MY_INSTALL_DIR
rm cmake-linux.sh
```

### **Install other required tools**

Install the basic tools required to build gRPC:

- **Linux**
    
    ```bash
    sudo apt install -y build-essential autoconf libtool pkg-config
    ```
    
- **macOS:**
    
    ```bash
    brew install autoconf automake libtool pkg-config
    ```
    

### **Clone the `grpc` repo**

Clone the `grpc` repo and its submodules:

```bash
cd ~
git clone --recurse-submodules -b v1.64.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
```

### **Build and install gRPC and Protocol Buffers**

While not mandatory, gRPC applications usually leverage [Protocol Buffers](https://developers.google.com/protocol-buffers) for service definitions and data serialization, and the example code uses [proto3](https://protobuf.dev/programming-guides/proto3).

The following commands build and locally install gRPC and Protocol Buffers:

```bash
cd grpc
mkdir -p cmake/build
pushd cmake/build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ../..
make -j 8
make install
popd
```
    

### Reference

https://grpc.io/docs/languages/cpp/quickstart/
