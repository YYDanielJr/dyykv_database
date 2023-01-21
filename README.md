# dyykv_database
## 使用方法：
直接运行dyykvdb文件，在有Threadpool ready的提示之后可以进行新的连接，连接端口为11451。<br>
如果需要关闭程序，确保焦点在服务器的运行终端，然后按下esc键。

## 编译方法：
运行build.sh文件。<br>
这个目录下确实存在makefile文件，但是我发现不论在wsl还是实体机的linux下，执行make都会自动略过DataBase.cpp不编译，所以不要尝试make命令。
