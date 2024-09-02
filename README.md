# gRPC를 활용한 콜백 함수 기반의 원격 YOLO 객체 인식 프로그램**

- **callback_client.cc**
    1. Conditional Variable, Mutex를 요청을 보내고 응답을 기다린다. Mutex를 사용하여 비동기적인 동작으로 보일 수 있지만 실제 동작 방식은 동기적으로 이루어진다.
- **callback_server.cc**
    1. 클라이언트의 요청을 동기적으로 처리하여 회신한다.

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
