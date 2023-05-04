# dyykv_database
## 目前的bug：
有小概率发生死锁。
## 使用方法：
直接运行dyykvdb文件，在有Threadpool ready的提示之后可以进行新的连接，连接端口为11451。<br>
如果需要关闭程序，确保焦点在服务器的运行终端，然后按下esc键。

## 编译方法：
先提前在Linux环境上安装好cmake（以debian系的Linux环境为例）：
```
sudo apt update
sudo apt install cmake
```
然后运行build.sh文件。<br>
