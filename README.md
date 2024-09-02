# gRPC를 활용한 비동기식 원격 YOLO 객체 인식 프로그램 - study

- **async_client.cc**
    1. gRPC 예제에서 참조한 코드임. 이름은 Async이지만 실제 코드는 Synchronous 코드임.
- **async_client2.cc**
    1. 동영상을 불러와 이미지를 추출함과 동시에 서버로 한 장씩 비동기적으로 송신한다.
    2. Bounding Box가 포함된 이미지를 서버로부터 비동기적으로 수신한다.
- **async_server.cc**
    1. 클라이언트로부터 수신한 이미지에서 YOLO를 사용하여 객체를 검출해낸다.
    2. 검출된 객체의 Bounding Box를 영상에 추가하여 회신한다.
    3. 이미지의 수신, 처리, 회신의 과정은 비동기적으로 진행되어 이미지가 들어오는 즉시 처리가 시작되고 완료되면 마자 회신한다.
    4. 비동기 구현은 **Completeion Queue**를 사용하여 CREATE -> PROCESS -> FINISH 순으로 요청을 처리한다.

### Clone and Build this Repository

```bash
cd ~
git clone --branch v2.2_Mat-Async https://github.com/YooJuno/study-gRPC.git
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
![alt text](images/image.png)    

### CLIENT

- ./remote_client   <VIDEO_PATH>   <CIRCLE:0 , YOLO:1>
```bash
mkdir ../result
./remote_client ../dataset/video.mp4 1
```
![alt text](images/image-1.png)


- You can see the processed picture by server

    ![alt text](images/image-2.png)

    

### Reference

https://grpc.io/docs/languages/cpp/quickstart/

https://github.com/improvess/yOLOv4-opencv-cpp-python
