# gRPC를 활용한 원격 영상처리 프로그램

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
