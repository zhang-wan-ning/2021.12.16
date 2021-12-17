#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <pthread.h>





#define PORT 8888
#define IP "192.168.1.11"

#define ERR_MSG(msg) do{\
	printf("__%d__ __%s__\n", __LINE__, __func__);\
	perror(msg);\
}while(0)



#define NAMELEN 16
#define DATALEN 128

#define ADMIN 1	//管理员
#define USER  0	//员工

#define STAFF_DATABASE 	 "staff_manage_system.db"

/*员工基本信息*/
typedef struct staff_info{
	int  no; 			//员工编号
	int  usertype;  	//ADMIN 1	USER 0	 
	char name[NAMELEN];	//姓名
	char passwd[8]; 	//密码
	int  age; 			// 年龄
	char phone[NAMELEN];//电话
	char addr[DATALEN]; // 地址
	char work[DATALEN]; //职位
	char date[DATALEN];	//入职年月
	int level;			// 等级
	double salary ;		// 工资
	
}staff_info_t;

struct cinmes{
	sqlite3* db;
	int newfd;
	struct sockaddr_in cin;

};


/*定义双方通信的结构体信息*/
typedef struct {

	int  msgtype;     //请求的消息类型
	int  usertype;    //ADMIN 1	USER 0	   
	int  id;          //工号
	char passwd[8];			 //登陆密码
	char recvmsg[DATALEN];   //通信的消息
	int  flags;      //标志位
	staff_info_t info;      //员工信息
}MSG;


enum fun{
	LOGIN, //员工登陆
	QUERY,  //查询员工信息
	MODIFY, //员工修改信息
	QUERY_ALL, //打印所有员工信息
	USER_QUIT, 	  //员工退出
	ADMIN_MODIFY, //管理员修改信息
	ADMIN_ADDUSER, //管理员添加成员
	ADMIN_DELUSER,  //管理员删除成员
	ADMIN_HISTORY,  //管理员查询历史记录
	QUIT, 			//退出

};

//服务器函数
void *handler(void *arg);
int login(MSG msg, sqlite3 *db, int newfd);  //员工登陆（共用）
int user_admin_query(MSG msg,sqlite3 *db,int newfd); //查询员工信息(共用)

int admin_adduser(int sfd, sqlite3 *db, MSG msg); //添加新员工
int admin_deluser(int sfd,sqlite3 *db,MSG msg);//删除员工
int modify(int newfd,sqlite3 *db,MSG msg); //管理员修改员工信息(共用)
int admin_history(int newfd,sqlite3 *db,MSG msg);  //查看历史记录
int admin_all_inf(int sfd,sqlite3 *db,MSG msg); //查询所有员工信息




char *get_time();   //获取时间函数


//客户端函数
int do_login(int sfd);//主界面
int do_user_login(int sfd);//员工登陆
int quit(MSG msg,sqlite3 *db,int newfd); //退出

int do_admin_login(int sfd); //管理员登陆界面
int admin_menu(int sfd,MSG msg); //管理员菜单

int add_user(int sfd,MSG msg); //添加新员工
int del_user(int sfd); //删除员工
int modify_user_inf(int sfd,MSG msg); //修改员工信息
int query_inf(int sfd); //查询员工信息
int query_history(int sfd); //查询历史记录
int query_all_inf(int sfd); //查询所有员工信息

int user_menu(int sfd,MSG msg);  //员工系统界面
int do_user_query(int sfd, MSG msg);//员工查序自身信息
int do_modification(int sfd, MSG msg);//员工修改自身信息

#endif


