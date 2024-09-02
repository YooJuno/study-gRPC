# gRPC를 활용한 파일 다운로드 프로그램**

- **remote_client.cc**
    1. 서버로부터 받은 파일 목록 중, 원하는 파일을 요청하여 Bytestream으로 변환된 protobuf 타입의 데이터 수신.
- **remote_server.cc**
    1. 클라이언트에게 파일 목록을 제공하고, 요청받은 파일에 대해 Bytestream으로 변환하여 송신.

### Clone and Build this Repository

## **Try it!**

### SERVER

- ./remote_server  <DATASET_FOLDER_PATH>

```bash
./remote_server ../dataset/
```


![Untitled](images/Untitled%207.png)

### CLIENT

- ./remote_client <DOWNLOAD_FOLDER_PATH>
```bash
mkdir ../download
./remote_client ../download/
```
    
- Choose number you wanna download.
    
    ![Untitled](images/Untitled%202.png)
    

- Then you can see the information of file you select.
    
    ![Untitled](images/Untitled%203.png)
    

- You can see the file in Download folder
    
    ![Untitled](images/Untitled%205.png)
    
    ![Untitled](images/Untitled%206.png)
    

### Reference

https://grpc.io/docs/languages/cpp/quickstart/
