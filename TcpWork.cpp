#include "include/TcpWork.h"

Connector::Connector(int domain, int type, int protocol, int max_link_in)
{
    // 默认构造和拷贝构造被禁用。
    // 有参构造传入值是socket类型和最大连接数。
    // 在这个构造函数里我们会完成三件事：
    // 1.初始化socket，得到sock_for_listening文件标识符；
    // 2.将这个监听文件标识符和本地ip绑定；
    // 3.设置监听。
    // 默认连接端口是11451。
    if_listening_open = false;
    default_pool = nullptr;
    pool_is_running = false;
    if_connection_open = false;
    max_link = max_link_in;
    std::string temp;
    sock_for_listening = socket(domain, type, protocol); // 初始化socket，得到监听文件标识符
    if (sock_for_listening == -1)
    {
        logger.error("Something went wrong when creating listening socket.");
        throw(SOCKET_CREATION_ERROR);
    }
    temp = "Created listening socket. Socket number is ";
    std::ostringstream transformer;
    transformer << temp << sock_for_listening << ".";
    temp = transformer.str();
    logger.info(temp);
    if_listening_open = true;
    
    addr.sin_family = domain;
    addr.sin_port = htons(11451); // 默认端口给到11451
    addr.sin_addr.s_addr = INADDR_ANY;
    int returner = bind(sock_for_listening, (sockaddr*)&addr, sizeof(addr)); // 绑定
    if (returner == -1)
    {
        logger.error("Something went wrong when binding listening socket with local IP.");
        throw(BIND_ERROR);
    }
    logger.info("Binded successfully. Connect to the server via port 11451.");
    returner = listen(sock_for_listening, max_link_in); // 设置监听
    if (returner == -1)
    {
        logger.error("Error occurs when setting listening. Make sure no process is using port 11451.");
        throw(LISTENING_SETTING_ERROR);
    }
    logger.info("Listening...");
}

void Connector::start_connection()
{
    if(!if_listening_open)
    {
        throw(LISTENING_STATUS_ERROR);
    }
    if_connection_open = true;
    logger.info("Started connections.");
    start_pool(max_link);
    logger.info("Threadpool ready.");
    while(if_connection_open)
    {
        client_info* client_new = new client_info;
        socklen_t addrlen = sizeof(sockaddr_in);
        client_new->sock_for_connection = accept(sock_for_listening, (sockaddr*)&(client_new->client_addr), &addrlen);
        if(client_new->sock_for_connection == -1)
        {
            throw(ACCEPT_ERROR);
            continue;
        }
        logger.info("A new connection has been accepted.");
        default_pool->push(client_resolver, client_new);
        logger.info("Connection pushed into threadpool successfully.");
    }
}

void client_resolver(client_info* client_infomation_ptr)
{
    std::ostringstream msg;
    char ip[24] = {0};
    msg << "Connected to client " << inet_ntop(AF_INET, &client_infomation_ptr->client_addr.sin_addr.s_addr, ip, sizeof(ip)) << ":" << ntohs(client_infomation_ptr->client_addr.sin_port) << ".";
    std::string strmsg = msg.str();
    logger.info(strmsg);
    while(1)
    {
        char buffer[1024] = {0};
        int returner = read(client_infomation_ptr->sock_for_connection, buffer, sizeof(buffer));
        //读进来的数据分为两部分，一部分是header，一部分是具体内容（称为包）。
        //前16个字节的内容是header。
        //header的前4个字节是一个unsigned int数字，固定为1234。
        //header的第二组4个字节是包的大小（不包括header），也是一个unsigned int数字。
        //header的第三组4个字节同样是一个unsigned int数，是本次传输的类型，从0到5，一共有6种，下面会具体说明。
        //header的第四组4字节全为0。
        //接收的时候需要填充16字节的header，给客户端回发的时候也需要有这16字节的header。
        
        bool buf2 = true;
        write(client_infomation_ptr->sock_for_connection, &buf2, returner);
    }
}
