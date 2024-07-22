<!-- # gRPC C++ Hello World Example

You can find a complete set of instructions for building gRPC and running the
Hello World app in the [C++ Quick Start][].

[C++ Quick Start]: https://grpc.io/docs/languages/cpp/quickstart
# study-gRPC

# Build Command
 -->

# gRPC STUDY

## 구현 환경

- OS : Ubuntu20.04.0 LTS, macOS SONOMA(v.14.5)

## Installation

### Setup

```bash
$ export MY_INSTALL_DIR=$HOME/.local
$ mkdir -p $MY_INSTALL_DIR
$ export PATH="$MY_INSTALL_DIR/bin:$PATH"
```

### **Install cmake**

You need version 3.13 or later of `cmake`. Install it by following these instructions:

- Linux
    
    ```bash
    $ sudo apt install -y cmake
    ```
    
- macOS:
    
    ```bash
    $ brew install cmake
    ```
    

Check the version of `cmake`:

```bash
$ cmake --version
cmake version 3.19.6
```

Under Linux, the version of the system-wide `cmake` can often be too old. You can install a more recent version into your local installation directory as follows:

```bash
$ wget -q -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.19.6/cmake-3.19.6-Linux-x86_64.sh
$ sh cmake-linux.sh -- --skip-license --prefix=$MY_INSTALL_DIR
$ rm cmake-linux.sh
```

### **Install other required tools**

Install the basic tools required to build gRPC:

- **Linux**
    
    ```bash
    $ sudo apt install -y build-essential autoconf libtool pkg-config
    ```
    
- **macOS:**
    
    ```bash
    $ brew install autoconf automake libtool pkg-config
    ```
    

### **Clone the `grpc` repo**

Clone the `grpc` repo and its submodules:

```bash
$ cd ~
$ git clone --recurse-submodules -b v1.64.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
```

### **Build and install gRPC and Protocol Buffers**

While not mandatory, gRPC applications usually leverage [Protocol Buffers](https://developers.google.com/protocol-buffers) for service definitions and data serialization, and the example code uses [proto3](https://protobuf.dev/programming-guides/proto3).

The following commands build and locally install gRPC and Protocol Buffers:

```bash
$ cd grpc
$ mkdir -p cmake/build
$ pushd cmake/build
$ cmake -DgRPC_INSTALL=ON **\**
      -DgRPC_BUILD_TESTS=OFF **\**
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR **\**
      ../..
$ make -j 4
$ make install
$ popd
```

### Clone and Build this Repository

```bash
cd ~/grpc/examples/cpp/
git clone https://github.com/YooJuno/study-gRPC
cd study-gRPC
mkdir -p cmake/build download
cd cmake/build
cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../
make
```

## **Try it!**

### SERVER

```bash
./remote_serv
```

- and then enter dataset path

![Untitled](gRPC%20STUDY%20a53fcbb9e66f4d159cb7dcc13b9fa2d7/Untitled.png)

### SERVER

```bash
./remote_client
```

- ID : juno , PW : 980220
    
    ![Untitled](gRPC%20STUDY%20a53fcbb9e66f4d159cb7dcc13b9fa2d7/Untitled%201.png)
    

- Choose number you wanna download.
    
    ![Untitled](gRPC%20STUDY%20a53fcbb9e66f4d159cb7dcc13b9fa2d7/Untitled%202.png)
    

- Then you can see the information of file you select.
    
    ![Untitled](gRPC%20STUDY%20a53fcbb9e66f4d159cb7dcc13b9fa2d7/Untitled%203.png)
    

- Finally, Enter the path of Download Folder
    
    ![Untitled](gRPC%20STUDY%20a53fcbb9e66f4d159cb7dcc13b9fa2d7/Untitled%204.png)
    

- You can see the picture in Download folder
    
    ![Untitled](gRPC%20STUDY%20a53fcbb9e66f4d159cb7dcc13b9fa2d7/Untitled%205.png)
    
    ![Untitled](gRPC%20STUDY%20a53fcbb9e66f4d159cb7dcc13b9fa2d7/Untitled%206.png)
    

### References

https://grpc.io/docs/languages/cpp/quickstart/