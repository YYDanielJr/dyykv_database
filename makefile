dyykvdb:TcpWork.o main.o Logger.o DataProcessor.o
	g++ main.o TcpWork.o Logger.o DataProcessor.o DataBase.o -o dyykvdb -lpthread

main.o:main.cpp
	g++ -c main.cpp -lpthread
Logger.o:Logger.cpp
	g++ -c Logger.cpp -lpthread
DataBase.o:DataBase.cpp
	g++ -c DataBase.cpp -lpthread
TcpWork.o:TcpWork.cpp
	g++ -c TcpWork.cpp -lpthread
DataProcessor.o:DataProcessor.cpp
	g++ -c DataProcessor.cpp -lpthread

.PHONY:
	rm TcpWork.o main.o Logger.o DataProcessor.o DataBase.o