# gRPC를 활용한 Mat(OpenCV) 송수신 프로그램

## Prerequisite
- Protocol Buuffer
- CMAKE
- gRPC
- YOLOv4



## OS

- Ubuntu 20.04 이상
- macOS 14

## Installation

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

### Clone and Build this Repository

```bash
cd ~
git clone --branch v2.1_Mat-Sync https://github.com/YooJuno/study-gRPC
cd study-gRPC
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local  ../CMakeLists -B .
make -j 8
```

## **Try it!**

### SERVER

- ./remote_server

```bash
./remote_server
```
![alt text](image.png)    

### CLIENT

- ./remote_client   <VIDEO_PATH>   <CIRCLE:0 , YOLO:1>
```bash
mkdir ../result
./remote_client ../dataset/video.mp4 1
```
![alt text](image-1.png)


- You can see the processed picture by server

    ![alt text](image-2.png)

    

### Reference

https://grpc.io/docs/languages/cpp/quickstart/

https://github.com/improvess/yOLOv4-opencv-cpp-python