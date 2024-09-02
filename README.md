# gRPC Study during GMD-SOFT internship period

## Versions
- ***Version 1.0***
    - **gRPC를 활용한 파일 다운로드 프로그램**
    - **CLIENT**
        1. 
    - **SERVER**
        1. 

    LINK : **https://github.com/YooJuno/study-gRPC/tree/v1.0_File-Download-Sync**

- ***Version 1.1***
    - **CLIENT**
        1.  
    - **SERVER**
        1.  
    
    LINK : **https://github.com/YooJuno/study-gRPC/tree/v1.1_File-Download-Sync-Stream**

- ***Version 2.0***
    - **gRPC를 활용한 동기식 원격 YOLO 객체 인식 프로그램**
    - **CLIENT**
        1.  
    - **SERVER**
        1.  
    
    LINK : **https://github.com/YooJuno/study-gRPC/tree/v2.0_Mat-Sync**

- ***Version 2.1***
    - **gRPC를 활용한 콜백 함수 기반의 원격 YOLO 객체 인식 프로그램**
    - **CLIENT**
        1.  
    - **SERVER**
        1.  
    
    LINK : **https://github.com/YooJuno/study-gRPC/tree/v2.1_Mat-Callback**

- ***Version2.2***
    - **gRPC를 활용한 비동기식 원격 YOLO 객체 인식 프로그램 - study**
    - **CLIENT**
        1. 동영상을 불러와 이미지를 추출함과 동시에 서버로 한 장 씩 비동기적으로 송신한다.
        2. Bounding Box가 포함된 이미지를 서버로부터 비동기적으로 수신한다.
    - **SERVER**
        1. 클라이언트로부터 수신한 이미지에서 YOLO를 사용하여 객체를 검출해낸다.
        2. 검출된 객체의 Bounding Box를 영상에 추가하여 회신한다.
        3. 이미지의 수신, 처리, 회신의 과정은 비동기적으로 진행되어 이미지가 들어오는 즉시 처리가 시작되고 완료되면 마자 회신한다.
        4. 비동기 구현은 **Completeion Queue**를 사용하여 CREATE -> PROCESS -> FINISH 순으로 요청을 처리한다.
    
    LINK : **https://github.com/YooJuno/study-gRPC/tree/v2.2_Mat-Async**

- ***Version3.0***
    - **gRPC를 활용한 비동기식 원격 YOLO 객체 인식 프로그램 - FINAL**
    - **CLIENT**
        1. 동영상을 불러와 이미지를 추출함과 동시에 서버로 한 장 씩 비동기적으로 송신한다.
        2. 또한, 비동기적으로 서버로부터 객체의 정보(**YoloData**)를 수신하여 객체 리스트에 저장한다.
        3. 모든 영상처리에 대한 수신이 끝나면 원본 동영상에 **YoloData**를 합쳐 재생 및 저장을 진행한다.
    - **SERVER**
        1. 클라이언트로부터 수신한 이미지에서 YOLO를 사용하여 객체를 검출해낸다.
        2. 검출된 객체의 정보인 **YoloData**를 회신한다.
        3. 이미지의 수신, 처리, 회신의 과정은 비동기적으로 진행되어 이미지가 들어오는 즉시 처리가 시작되고 완료되면 회신한다.
        4. 비동기 구현은 **Completeion Queue**를 사용하여 CREATE -> PROCESS -> FINISH 순으로 요청을 처리한다.
    
    LINK : **https://github.com/YooJuno/study-gRPC/tree/v3.0_gRPC-Final**


## OS
- Linux , MacOS

## Libraries
- OpenCV4.4 ~
- Protocol buffer

# Installation gRPC

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
