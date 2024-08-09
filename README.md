# gRPC를 활용한 파일 송수신 프로그램

### Clone and Build this Repository

```bash
cd ~
git clone --branch v1.0_File-Download-Sync https://github.com/YooJuno/study-gRPC.git
cd study-gRPC
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local  ..
make -j 8
```

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
