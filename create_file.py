def create_file(file_name, size_in_bytes):
    with open(file_name, 'wb') as f:
        f.write(b'\0' * size_in_bytes)

# 예제 사용법: 10MB 크기의 파일 생성
size_in_megabytes = 500
file_name = 'test_file(' + str(size_in_megabytes) +'MB).dat'
size_in_bytes = size_in_megabytes * 1024 * 1024

create_file(file_name, size_in_bytes)
print(f'{file_name} 파일이 생성되었습니다. 크기: {size_in_megabytes} MB')
