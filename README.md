# gRPC를 활용한 동기식 원격 YOLO 객체 인식 프로그램**

- **remote_client.cc**
    1. 동기적으로 이미지를 송신하고 서버의 응답(ProtoMat)을 기다림.
- **remote_server.cc**
    1. 동기적으로 이미지를 수신하고 영상 처리 후에 회신.

### Clone and Build this Repository

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
