# gRPC를 활용한 원격 영상처리 프로그램

### Clone and Build this Repository

```bash
cd ~
git clone --branch v3.0_gRPC-Final https://github.com/YooJuno/study-gRPC.git
cd study-gRPC
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local  ../CMakeLists.txt -B .
make -j 8
```

## **Try it!**

### SERVER

- ./remote_server

```bash
./remote_server
```
![alt text](images/image.png)    

### CLIENT

- ./remote_client   <VIDEO_PATH>   <CIRCLE:0 , YOLO:1>
```bash
./remote_client
```
![alt text](images/image-1.png)


- You can see the processed picture by server

    ![alt text](images/image-2.png)

    

### Reference

https://grpc.io/docs/languages/cpp/quickstart/

https://github.com/improvess/yOLOv4-opencv-cpp-python
