/*
 * @Description: 
 * @Author: zxl
 * @Date: 2024-05-05 15:44:51
 * @FilePath: /chatting-room/client.c
 * @LastEditTime: 2024-05-05 16:52:42
 * @LastEditors: zxl
 */
 /*
  * FILE: client.c
  * DATE: 20180206
  * ==============
  */
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/socket.h>   
 #include <arpa/inet.h>   
 #include <netinet/in.h>
 #include <pthread.h>
 #include <unistd.h>
 #define BUFFSIZE 128
 #define HOST_IP "192.168.94.129"
 #define PORT 8000
 
 int sockfd;
 
 void* snd(void*arg);
 
 int main()
 {
         pthread_t thread;       // pthread_t 线程，gcc编译时需加上-lpthread
         struct sockaddr_in serv_addr;   // struct sockaddr_in
         char buf[BUFFSIZE];
         // 初始化服务端地址结构
         bzero(&serv_addr, sizeof(struct sockaddr_in));  // bzero 清零
         serv_addr.sin_family = AF_INET;         // sin_family   AF_INET
         serv_addr.sin_port = htons(PORT);       // sin_port     htons(PORT)
         if(inet_pton(AF_INET6,HOST_IP, &serv_addr.sin_addr)==-1)
         {
                perror("ip convert errror");
                exit(-1);
         }    // inet_pton
         // 创建客户端套接字
         sockfd = socket(AF_INET, SOCK_STREAM, 0);       // socket 创建套接字
         if(sockfd < 0)
         {
                 perror("fail to socket");
                 exit(-1);
         }
         // 与服务器建立连接
         printf("connecting... \n");
         if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // connect
         {
                 perror("fail to connect");
                 exit(-2);
         }
         /* === 从此处开始 程序分做两个线程 === */
         // 创建发送消息的线程，调用发送消息的函数snd
         pthread_create(&thread, NULL, (void *)(&snd), NULL);    // pthread_create
         // 接收消息的线程
         while(1)
         {
                 int len;
                 if((len=read(sockfd, buf, BUFFSIZE)) > 0)       // read 读取通信套接字
                 {
                         buf[len] = '\0';        // 添加结束符，避免显示缓冲区中残留的内容
                         printf("\n%s", buf);
                         fflush(stdout);         // fflush 冲洗标准输出，确保内容及时显示
                 }
         }
         return 0;
 }
 
 // 发送消息的函数
 void* snd(void*arg)
 {
         char name[32], buf[BUFFSIZE];
 
         fgets(name, 32, stdin); // fgets 会读取输入字符串后的换行符
         write(sockfd, name, strlen(name));      // write 写入通信套接字
         while(1)
         {
                 fgets(buf, BUFFSIZE, stdin);
                 write(sockfd, buf, strlen(buf));
                 if(strcmp(buf, "bye\n") == 0)   // 注意此处的\n
                         exit(0);
         }
 }