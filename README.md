# gRPC를 활용한 원격 영상 처리 프로그램

## **Clone & Build this Repository**

```
cd ~
git clone --branch v3.0_gRPC-Final https://github.com/YooJuno/study-gRPC.git
cd study-gRPC
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local  ../CMakeLists.txt -B .
make -j 8
```

# **How it works**
1. Client sends just image continueously from video.
2. Server receives image and process YOLO with.
3. Server doesn't return processed image, but metadata of detected objects information such as x,y width, height...
4. With metadata of YOLO from server, client merge it to original image and create new video which contains YOLO bounding boxes in every images.

# **Try it!**

## **SERVER**

```bash
./remote_server
```

![image.png](images/image-4.png)

## **CLIENT**

### **./remote_client    --input_video_path=<…>     --output_video_path=<…>**

```
./async_client
```

- You don’t need to put arguments. It has a default values for args.

## Result

!https://github.com/YooJuno/study-gRPC/raw/v3.0_gRPC-Final/images/image-1.png

![image.png](images/image-2.png)

![image.png](images/image-1.png)
- Press ESC key if you wanna quit player.

![image.png](images/image-3.png)



**Reference**

https://grpc.io/docs/languages/cpp/quickstart/

https://github.com/improvess/yOLOv4-opencv-cpp-python