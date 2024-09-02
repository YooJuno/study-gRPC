# gRPC Study during GMD-SOFT internship period

## Versions

- ***Version 1.0***
    [**=> LINK**](https://github.com/YooJuno/study-gRPC/tree/v1.0_File-Download-Sync)

    **gRPC를 활용한 파일 다운로드 프로그램**
    - **remote_client.cc**
        1. 서버로부터 받은 파일 목록 중, 원하는 파일을 요청하여 Bytestream으로 변환된 protobuf 타입의 데이터 수신.
    - **remote_server.cc**
        1. 클라이언트에게 파일 목록을 제공하고, 요청받은 파일에 대해 Bytestream으로 변환하여 송신.

- ***Version 1.1***
    [**=> LINK**](https://github.com/YooJuno/study-gRPC/tree/v1.1_File-Download-Sync-Stream)

    **gRPC를 활용한 Stream 방식의 대용량 파일 다운로드 프로그램**
    - **remote_client.cc**
        1. 서버로부터 받은 파일 목록 중, 원하는 파일을 요청하여 chunk 단위로 나눠서 수신.
    - **remote_server.cc**
        1. 클라이언트에게 파일 목록을 제공하고, 요청받은 파일에 대해 chunk 단위로 나눠 전송.

- ***Version 2.0***
    [**=> LINK**](https://github.com/YooJuno/study-gRPC/tree/v2.0_Mat-Sync)

    **gRPC를 활용한 동기식 원격 YOLO 객체 인식 프로그램**
    - **remote_client.cc**
        1. 동기적으로 이미지를 송신하고 서버의 응답(ProtoMat)을 기다림.
    - **remote_server.cc**
        1. 동기적으로 이미지를 수신하고 영상 처리 후에 회신.

- ***Version 2.1***
    [**=> LINK**](https://github.com/YooJuno/study-gRPC/tree/v2.1_Mat-Callback)

    **gRPC를 활용한 콜백 함수 기반의 원격 YOLO 객체 인식 프로그램**
    - **callback_client.cc**
        1. Conditional Variable, Mutex를 요청을 보내고 응답을 기다린다. Mutex를 사용하여 비동기적인 동작으로 보일 수 있지만 실제 동작 방식은 동기적으로 이루어진다.
    - **callback_server.cc**
        1. 클라이언트의 요청을 동기적으로 처리하여 회신한다.

- ***Version2.2***
    [**=> LINK**](https://github.com/YooJuno/study-gRPC/tree/v2.2_Mat-Async)

    **gRPC를 활용한 비동기식 원격 YOLO 객체 인식 프로그램 - study**
    - **async_client.cc**
        1. gRPC 예제에서 참조한 코드임. 이름은 Async이지만 실제 코드는 Synchronous 코드임.
    - **async_client2.cc**
        1. 동영상을 불러와 이미지를 추출함과 동시에 서버로 한 장씩 비동기적으로 송신한다.
        2. Bounding Box가 포함된 이미지를 서버로부터 비동기적으로 수신한다.
    - **async_server.cc**
        1. 클라이언트로부터 수신한 이미지에서 YOLO를 사용하여 객체를 검출해낸다.
        2. 검출된 객체의 Bounding Box를 영상에 추가하여 회신한다.
        3. 이미지의 수신, 처리, 회신의 과정은 비동기적으로 진행되어 이미지가 들어오는 즉시 처리가 시작되고 완료되면 회신한다.
        4. 비동기 구현은 **Completeion Queue**를 사용하여 CREATE -> PROCESS -> FINISH 순으로 요청을 처리한다.

- ***Version3.0***
    [**=> LINK**](https://github.com/YooJuno/study-gRPC/tree/v3.0_gRPC-Final)

    **gRPC를 활용한 비동기식 원격 YOLO 객체 인식 프로그램 - FINAL**
    - **async_client.cc**
        1. 동영상을 불러와 이미지를 추출함과 동시에 서버로 한 장씩 비동기적으로 송신한다.
        2. 또한, 비동기적으로 서버로부터 객체의 정보(**YoloData**)를 수신하여 객체 리스트에 저장한다.
        3. 모든 영상처리 결과에 대한 수신이 끝나면 원본 동영상에 **YoloData**를 합쳐 재생 및 저장을 진행한다.
    - **async_server.cc**
        1. 클라이언트로부터 수신한 이미지에서 YOLO를 사용하여 객체를 검출해낸다.
        2. 검출된 객체의 정보인 **YoloData**를 회신한다.
        3. 이미지의 수신, 처리, 회신의 과정은 비동기적으로 진행되어 이미지가 들어오는 즉시 처리가 시작되고 완료되면 회신한다.
        4. 비동기 구현은 **Completeion Queue**를 사용하여 CREATE -> PROCESS -> FINISH 순으로 요청을 처리한다.

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

- https://grpc.io/docs/languages/cpp/quickstart/
