#include "common.h"
int main(int argc, const char *argv[])
{
	
	//创建网络通信的套接字
	int sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd < 0){
		ERR_MSG("socket");
		return -1;
	}

	//填充网络结构体


	struct sockaddr_in sin;
	sin.sin_family 		= AF_INET;
	sin.sin_port 		= htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);

	//连接服务器
	if(connect(sfd,(struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		ERR_MSG("connect");
		return -1;
	}

	do_login(sfd);

	close(sfd);

	return 0;
}




/************************************************
 *函数名：do_login
 *参   数：套接字、消息结构体
 *返回值：是否登陆成功
 *功   能：登陆
 *************************************************/
int do_login(int sfd)
{	
	int n;
	MSG msg;

	while(1){
		printf("*************************************************************\n");
		printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			do_admin_login(sfd);
			break;
		case 2:
			do_user_login(sfd);
			break;
		case 3:
			close(sfd);
			exit(0);
		default:
			printf("您的输入有误，请重新输入\n"); 
			//continue;
		}
		//printf("输入回车清屏\n");
		//system("clear");
	}
	return 0;

}

int do_user_login(int sfd) //员工登陆
{
	MSG msg;
	char buf[1024] = "";
	msg.msgtype = LOGIN;
	msg.usertype = USER;


	//输入用户名和密码
	printf("请输入您的工号>>");
	scanf("%d",&msg.id);
	getchar();
	
	memset(msg.passwd, 0, DATALEN);
	printf("请输入密码>>");
	scanf("%s",msg.passwd);
	getchar();

	//发送登陆请求
	send(sfd, &msg, sizeof(msg), 0);

	bzero(buf,sizeof(buf));
	//接受服务器响应
	recv(sfd, buf, sizeof(msg), 0);

	if(strncasecmp(buf,"***",3) == 0)
	{
		printf("登陆成功\n");
		user_menu(sfd,msg);
	}
	else{
		printf("%s\n",buf);
	}


	return 0;
}


int do_admin_login(int sfd) //登录
{
	MSG msg;
	char buf[1024] = "";
	msg.msgtype = LOGIN;
	msg.usertype = ADMIN;


	//输入用户名和密码
	printf("请输入您的工号>>");
	scanf("%d",&msg.id);
	getchar();

	memset(msg.passwd, 0, DATALEN);
	printf("请输入密码>>");
	scanf("%s",msg.passwd);
	getchar();

	//发送登陆请求
	send(sfd, &msg, sizeof(msg), 0);
	bzero(buf,sizeof(buf));

	//接受服务器响应
	recv(sfd, buf, sizeof(msg), 0);

	if(strncasecmp(buf,"***",3) == 0)
	{
		printf("登陆成功\n");
		admin_menu(sfd,msg);
	}
	else{
		printf("%s\n",buf);
	}

	return 0;
}


int user_menu(int sfd,MSG msg) //员工菜单
{
	int n = 0;
	while(1)
	{
		printf("-----欢迎登录员工系统-----\n");

		printf("-----1.查询自身信息-------\n");

		printf("-----2.修改自身信息-------\n");

		printf("-----3.返回登陆界面-------\n");

		printf("请输入您的选择>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			do_user_query(sfd,msg);
			break;
		case 2:	
 			do_modification(sfd,msg);
			break;
		case 3:
			msg.msgtype = QUIT;
			if(send(sfd, &msg, sizeof(MSG), 0)<0)
			{
				perror("do_login send");
				return -1;
			}
			return 0;
		default:
			printf("您输入的选择不正确，请重新输入\n");
			break;
		}
	}
	return 0;
}


int do_user_query(int sfd, MSG msg)//员工查序自身信息
{
	char buf[1024] = "";
	msg.msgtype = QUERY;

	send(sfd, &msg, sizeof(msg), 0);

	bzero(buf,sizeof(buf));
	recv(sfd, buf, sizeof(buf), 0);

	printf("staffno\tusertype\tstate\tname\tpasswd\tage\tphone\taddr\twork\tdate\tlevel\tsalary\n");
	printf("%s\n",buf);

	return 0;
}


int do_modification(int sfd, MSG msg)//员工修改自身信息
{
	char buf[1024] = "";
	msg.msgtype = MODIFY;
	msg.info.no = msg.id;
	int chose;
	
	send(sfd, &msg, sizeof(msg), 0);
	recv(sfd, &buf, sizeof(msg), 0);


	printf("您想修改他的什么信息：1.usertype 2.name 3.passwod 4.age 5.phoone \
			6.add 7.work 8.date 9.level 10.salary \n");
	scanf("%d",&msg.flags);
	getchar();
	chose = msg.flags;

	if (chose == 1|| chose == 4|| chose == 9||chose ==10)
	{

		printf("please input new 1information:");
		scanf("%d",&msg.info.age);
		getchar();

	}
	else
	{
		printf("please input new 2information:");
		scanf("%s",msg.info.addr);
		getchar();
	}

	send(sfd,&msg,sizeof(msg),0);
	
	bzero(buf,sizeof(buf));
	recv(sfd, buf, sizeof(buf), 0);

	printf("%s\n",buf);

	return 0;


}




int admin_menu(int sfd,MSG msg) //管理员菜单
{
	int n = 0;
	while(1)
	{
		printf("-----欢迎登录管理员系统-----\n\n");

		printf("-----1.添加新员工-----------\n");

		printf("-----2.删除员工-------------\n");

		printf("-----3.修改员工信息---------\n");

		printf("-----4.查询员工信息---------\n");

		printf("-----5.查询员工历史记录-----\n");

		printf("-----6.查询所有员工信息-----\n");

		printf("-----7.返回登陆界面---------\n");

		printf("请输入您的选择>>");
		scanf("%d",&n);
		getchar();

		switch(n)
		{
		case 1:
			add_user(sfd,msg);
			break;
		case 2:
			del_user(sfd);
			break;
		case 3:
			modify_user_inf(sfd,msg);
			break;
		case 4:
			query_inf(sfd);
			break;
		case 5:
			query_history(sfd);
			break;
		case 6:
			query_all_inf(sfd);
			break;
		case 7:
			msg.msgtype = QUIT;
			if(send(sfd, &msg, sizeof(MSG), 0)<0)
			{
				perror("do_login send");
				return -1;
			}
			return 0;
		default:
			printf("您输入的选择不正确，请重新输入\n");
			break;
		}
	}
	return 0;
}

int add_user(int sfd,MSG msg) //添加新员工
{
	int chose = 0;
	char buf[128] = "";
	msg.msgtype = ADMIN_ADDUSER;

loop:
	printf("您要添加的是：1.员工 2:管理员\n");
	scanf("%d",&chose);
	getchar();

	if (chose == 1)
	{
		msg.info.usertype = USER;

	}
	else if (chose == 2)
	{
		msg.info.usertype = ADMIN;
	}
	else 
	{
		printf("您输入的不正确,请重新输入！\n");
		goto loop;
	}
	printf("请输入他的工号>>>");
	scanf("%d",&msg.info.no);
	getchar();
	printf("请输入他的姓名>>>");
	scanf("%s",msg.info.name);
	getchar();
	printf("请输入他的密码(八位以内)>>>");
	scanf("%s",msg.info.passwd);
	getchar();
	printf("请输入他的年龄>>>");
	scanf("%d",&msg.info.age);
	getchar();
	printf("请输入他的电话>>>");
	scanf("%s",msg.info.phone);
	getchar();
	printf("请输入他的地址>>>");
	scanf("%s",msg.info.addr);
	getchar();
	printf("请输入他的职位>>>");
	scanf("%s",msg.info.work);
	getchar();
	printf("请输入他的入职年月>>>");
	scanf("%s",msg.info.date);
	getchar();
	printf("请输入他的等级>>>");
	scanf("%d",&msg.info.level);
	getchar();
	printf("请输入他的工资>>>");
	scanf("%lf",&msg.info.salary);
	getchar();

	send(sfd, &msg, sizeof(msg), 0);

	bzero(buf,sizeof(buf));
	recv(sfd, buf, sizeof(msg), 0);
	printf("%s\n",buf);


	return 0;
}

int del_user(int sfd)//删除员工
{
	MSG msg;
	char buf[128] = "";
	msg.msgtype = ADMIN_DELUSER;
	
	printf("请输入您想删除员工的工号>>>");
	scanf("%d",&msg.id);
	getchar();

	send(sfd, &msg, sizeof(msg), 0);

	bzero(buf,sizeof(buf));
	recv(sfd, buf, sizeof(msg), 0);

	printf("%s\n",buf);
	
	return 0;
}

int modify_user_inf(int sfd,MSG msg) //修改员工信息
{
	msg.msgtype = MODIFY;
	char buf[1024] = "";
	int chose = 0;
	int b = 0;

	printf("请输入你想修改的员工的id>>>");
	scanf("%d",&msg.info.no);
	getchar();

	send(sfd, &msg, sizeof(msg), 0);

	bzero(buf,sizeof(buf));
	recv(sfd, buf, sizeof(buf), 0);
	printf("%s\n",buf);
	if (strncasecmp(buf,"***",3) != 0)
	{
		printf("您输入的id不正确，或者没有该员工！\n");
		
		return -1;
	}

	printf("您想修改他的什么信息：1.usertype 2.name 3.passwod 4.age 5.phoone \
			6.add 7.work 8.date 9.level 10.salary 11.all\n");
	scanf("%d",&msg.flags);
	getchar();
	chose = msg.flags;

	if (chose == 11)
	{
		printf("请输入他的姓名>>>");
		scanf("%s",msg.info.name);
		getchar();
		printf("请输入他的密码(八位以内)>>>");
		scanf("%s",msg.info.passwd);
		getchar();
		printf("请输入他的新身份(0.员工 1.管理员 )>>>");
		scanf("%d",&msg.info.usertype);
		getchar();
		printf("请输入他的年龄>>>");
		scanf("%d",&msg.info.age);
		getchar();
		printf("请输入他的电话>>>");
		scanf("%s",msg.info.phone);
		getchar();
		printf("请输入他的地址>>>");
		scanf("%s",msg.info.addr);
		getchar();
		printf("请输入他的职位>>>");
		scanf("%s",msg.info.work);
		getchar();
		printf("请输入他的入职年月>>>");
		scanf("%s",msg.info.date);int admin_modify(int newfd,sqlite3 *db,MSG msg); 
		getchar();
		printf("请输入他的等级>>>");
		scanf("%d",&msg.info.level);
		getchar();
		printf("请输入他的工资>>>");
		scanf("%lf",&msg.info.salary);
		getchar();

	}
	else if (chose == 1|| chose == 4|| chose == 9||chose ==10)
	{

		printf("please input new 1information:");
		scanf("%d",&msg.info.age);
		getchar();

	}
	else
	{
		printf("please input new 2information:");
		scanf("%s",msg.info.addr);
		getchar();
	}

	send(sfd,&msg,sizeof(msg),0);
	
	bzero(buf,sizeof(buf));
	recv(sfd, buf, sizeof(buf), 0);

	printf("%s\n",buf);

	return 0;


}

int query_inf(int sfd) //查询员工信息
{
	MSG msg;
	msg.msgtype = QUERY;
	char buf[1024] = "";

	printf("请输入您想查询的员工的工号>>>");
	scanf("%d",&msg.id);
	getchar();

	if (send(sfd,&msg,sizeof(msg),0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	bzero(buf,sizeof(buf));
	if (recv(sfd,buf,sizeof(buf),0) < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	if (strncasecmp(buf,"***",3) != 0)
	{
		printf("staffno\tusertype\tstate\tname\tpasswd\tage\tphone\taddr\twork\tdate\tlevel\tsalary\n");
	}
	printf("%s\n",buf);

	return 0;
}


int query_history(int sfd) //查询历史记录
{
	MSG msg;
	msg.msgtype = ADMIN_HISTORY;
	char buf[1024] = "";

	if (send(sfd,&msg,sizeof(msg),0) < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	bzero(buf,sizeof(buf));


	printf("************************************\n");
	while(1)
	{
		bzero(buf,sizeof(buf));
		if (recv(sfd,buf,sizeof(buf),0) < 0)
		{
			ERR_MSG("recv");
			return -1;
		}
		if(strncasecmp(buf,"***",3) == 0)
		{
			break;
		}
		printf("%s\n",buf);
	}

	printf("***These are all historical records***\n");

	return 0;

}


int query_all_inf(int sfd) //查询所有员工信息
{
	MSG msg;
	char buf[128] = "";
	msg.msgtype = QUERY_ALL;
	int n = 0;

	send(sfd, &msg, sizeof(msg), 0);
	printf("staffno\tusertype\tstate\tname\tpasswd\tage\tphone\taddr\twork\tdate\tlevel\tsalary\n");

	while(1)
	{
		memset(buf,0,sizeof(buf));
		recv(sfd, buf, sizeof(buf), 0);
		printf("%s\n",buf);

		if (strncasecmp(buf,"***",3) == 0)
		{
			break;
		}
	}
	printf("-----Those are all employees-----\n");
	return 0;

}

