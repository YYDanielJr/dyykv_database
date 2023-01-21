g++ -c DataBase.cpp -lpthread
g++ -c Logger.cpp -lpthread
g++ -c DataProcessor.cpp -lpthread
g++ -c TcpWork.cpp -lpthread
g++ -c main.cpp -lpthread
g++ main.o TcpWork.o Logger.o DataProcessor.o DataBase.o -o dyykvdb -lpthread
rm TcpWork.o main.o Logger.o DataProcessor.o DataBase.o
