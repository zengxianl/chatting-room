 #include <stdio.h>
 #include <stdlib.h>     // exit
 #include <string.h>
 #include <unistd.h>     // bind listen
 #include <time.h>       // time(NULL)   ctime(&ticks)
 #include <netinet/in.h>
 #include <arpa/inet.h>  // 必须包含，用于inet_ntop
 #include <pthread.h>

 #define PORT 8000
 #define MAXMEM 10 
 #define BUFFSIZE 128
 
 #define DEBUG_PRINT 1         // 宏定义 调试开关
 #ifdef DEBUG_PRINT
 #define DEBUG(format, ...) printf("FILE: "__FILE__", LINE: %d: "format"\n", __LINE__, ##__VA_ARGS__)
 #else
 #define DEBUG(format, ...)
 #endif
 
 int listenfd, connfd[MAXMEM];
 
 void* quit(void*arg);
 void* rcv_snd(void*arg);
 
 int main()
 {
         struct sockaddr_in serv_addr, cli_addr;
 //      int len = sizeof(cli_addr), i;
         int i;
         time_t ticks;
         pthread_t thread;
         char buff[BUFFSIZE];
 
         printf("running...\n(Prompt: enter command ""quit"" to exit server)\n");
         DEBUG("=== initialize...");     // 初始化填充服务端地址结构
         bzero(&serv_addr, sizeof(struct sockaddr_in));
         serv_addr.sin_family = AF_INET;
         serv_addr.sin_port = htons(PORT);
         serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
 
         DEBUG("=== socket..."); // socket 创建服务器端的监听套接字
         listenfd = socket(AF_INET, SOCK_STREAM, 0);
         if(listenfd < 0)
         {
                 perror("fail to socket");
                 exit(-1);
         }
 
         DEBUG("=== bind...");   // bind 将套接字与填充好的地址结构进行绑定
         if(bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
         {
                 perror("fail to bind");
                 exit(-2);
         }
 
         DEBUG("=== listening..."); // listen 将主动连接套接字变为被动倾听套接字
         listen(listenfd, MAXMEM);
 
         /* === 创建一个线程，对服务器程序进行管理，调用quit函数 === */
         pthread_create(&thread, NULL, quit, NULL);
 
         // 将套接字描述符数组初始化为-1，表示空闲
         for(i=0; i<MAXMEM; i++)
                 connfd[i] = -1;
 
        while(1)
         {
                int len;// = sizeof(cli_addr);
                 for(i=0; i<MAXMEM; i++)
                 {
                         if(connfd[i] == -1)
                                 break;
                 }
                 // accept 从listen接受的连接队列中取得一个连接
                 connfd[i] = accept(listenfd, (struct sockaddr *)&cli_addr, (socklen_t*)&len);
                 if(connfd[i] < 0)
                 {
                         perror("fail to accept");
                 //      continue;       // 此句可以不用，accept会阻塞等待
                 }
                 ticks = time(NULL);
                 //sprintf(buff, "%.24s\r\n", ctime(&ticks));
                 printf("%.24s\n\tconnect from: %s, port %d\n",
                                 ctime(&ticks), inet_ntop(AF_INET, &(cli_addr.sin_addr), buff, BUFFSIZE),
                                 ntohs(cli_addr.sin_port));      // 注意 inet_ntop的使用，#include <arpa/inet.h>
 
                 /* === 针对当前套接字创建一个线程，对当前套接字的消息进行处理 === */
                 printf("before thread\n");
                 pthread_create((pthread_t*)malloc(sizeof(pthread_t)), NULL, rcv_snd, &i);
                 printf("after thread\n");

         }
         return 0;
 }
 
void* quit(void*arg)
 {
         char msg[10];
         while(1)
         {
                printf("quit on");
                 scanf("%s", msg);       // scanf 不同于fgets, 它不会读入最后输入的换行符
                 if(strcmp(msg, "quit") == 0)
                 {
                         printf("Byebye... \n");
                         close(listenfd);
                         exit(0);
                 }
         }
 }
 
 void* rcv_snd(void*arg)
 {      
        printf("in rcv");
        int n=*(int*)arg; 
         int len, i;
         char name[32], mytime[32], buf[BUFFSIZE];
         time_t ticks;
         int ret;
 
         // 获取此线程对应的套接字用户的名字
         write(connfd[n], "your name: ", strlen("your name: "));
         len = read(connfd[n], name, 32);
         if(len > 0)
                 name[len-1] = '\0';     // 去除换行符
         strcpy(buf, name);
         strcat(buf, "\tjoin in\n\0");
         // 把当前用户的加入 告知所有用户
         for(i=0; i<MAXMEM; i++)
         {
                 if(connfd[i] != -1)
                         write(connfd[i], buf, strlen(buf));
         }
 
         while(1)
         {
                printf("listening");
                 char temp[BUFFSIZE];
                 if((len=read(connfd[n], temp, BUFFSIZE)) > 0)
                 {
                         temp[len-1] = '\0';
                         // 当用户输入bye时，当前用户退出
                         if(strcmp(temp, "bye") == 0)
                         {
                                 close(connfd[n]);
                                 connfd[n] = -1;
                                 pthread_exit(&ret);
                         }
                         ticks = time(NULL);
                         sprintf(mytime, "%.24s\r\n", ctime(&ticks));
                         //write(connfd[n], mytime, strlen(mytime));
                         strcpy(buf, name);
                         strcat(buf, "\t");
                         strcat(buf, mytime);
                         strcat(buf, "\r\t");
                         strcat(buf, temp);
                         strcat(buf, "\n");
 
                         for(i=0; i<MAXMEM; i++)
                         {
                                 if(connfd[i] != -1)
                                         write(connfd[i], buf, strlen(buf));
                         }
                 }
         }
 
 }