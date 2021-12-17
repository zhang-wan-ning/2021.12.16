
#include "common.h"


int main(int argc, const char *argv[])
{

	//socket->填充->绑定->监听->等待连接->数据交互->关闭

	//创建网络通信的套接字
	int sfd = socket(AF_INET,SOCK_STREAM,0);
	if(sfd < 0){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sfd = %d套接字创建成功\n",sfd);

	//允许端口快速重用
	int reuse = 1;
	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <0)
	{
		perror("setsockopt");
		return -1;
	}
	
	//填充网络结构体
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port  = htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);	

	//绑定网络套接字和网络结构体
	if(bind(sfd,(struct sockaddr*)&sin, sizeof(sin)) < 0){
		printf("bind failed.\n");
		exit(-1);
	}
	printf("bind成功\n");

	//监听套接字，将主动套接字转化为被动套接字
	if(listen(sfd,15) < 0){
		printf("listen failed.\n");
		exit(-1);
	}
	printf("listen 成功\n");

	//打开数据库
	sqlite3 *db = NULL;

	if (sqlite3_open("./staff_manage_system.db",&db) != SQLITE_OK)
	{
		perror("数据库打开失败\n");
		printf("%s\n",sqlite3_errmsg(db));
		return -1;
	}
	printf("打开数据库成功\n");


	struct sockaddr_in cin;
	socklen_t addrlen = sizeof(cin);
	int newfd;
	pthread_t tid;
	struct cinmes cliinfo;

	while(1){

		//主线程负责链接
		newfd = accept(sfd, (struct sockaddr*)&cin, &addrlen);
		if (newfd < 0 )
		{
			ERR_MSG("accpet");
			return -1;
		}

		cliinfo.db = db;
		cliinfo.newfd = newfd;
		cliinfo.cin = cin;
		//一旦链接成功创建一个线程，用于和客户端交互

		if (pthread_create(&tid,NULL,handler,(void *)&cliinfo) != 0)
		{
			ERR_MSG("pthread_create");
			return -1;
		}
	}

	close(sfd);

	return 0;
}


void *handler(void *arg)
{
	//分离线程
	pthread_detach(pthread_self());

	struct cinmes cliinfo = *(struct cinmes *)arg;
	int newfd = cliinfo.newfd;
	sqlite3 *db = cliinfo.db;
	struct sockaddr_in cin = cliinfo.cin;

	ssize_t ret;
	MSG msg;

	while(1)
	{
		ret = recv(newfd,&msg,sizeof(msg),0);
		if (ret < 0)
		{
			ERR_MSG("recv");
		}
			else if (ret == 0)
		{
			//调用退出函数
			quit(msg,db,newfd);
			printf("客户端newfd=%d退出\n",newfd);
			break;
		}
		printf("接收数据成功\n");

		switch(msg.msgtype)
		{
		case LOGIN:
			login(msg,db,newfd);
			break;
		case QUERY:
			user_admin_query(msg,db,newfd);
			break;
		case QUERY_ALL:
			admin_all_inf(newfd,db,msg);
			break;
		case MODIFY:
			modify(newfd,db,msg);
			break;
		case ADMIN_ADDUSER:
			admin_adduser(newfd,db,msg);
			break;
		case ADMIN_DELUSER:
			admin_deluser(newfd,db,msg);
			break;
		case ADMIN_HISTORY:
			admin_history(newfd,db,msg);
			break;
		case QUIT:
			quit(msg,db,newfd);
			break;
		default:
			printf("输入错误，请重新输入\n");
			continue;
		}
	}
	close(newfd);
	pthread_exit(NULL);

}


int login(MSG msg, sqlite3 *db, int newfd)//登陆
{
	char sql1[1024] = "select * from usrinfo";
	char** result = NULL;
	char* errmsg = NULL;
	int row, column;

	int flag = 0;
	int line = 0;

	char buf[128] = "";


	if (sqlite3_get_table(db, sql1, &result, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ errmsg:%s\n", __LINE__,errmsg);
		return -1;
	}

	bzero(buf,sizeof(buf));



	for (line = 12;line<=row*column; line+=12)
	{

		if ((atoi(result[line])) == msg.id)
		{
			flag = 1;
			break;
		}
	}

	printf("1\n");
	if (flag == 0)
	{
		sprintf(buf,"账号不存在,请联系管理员注册");
		printf("%s\n",buf);
		if (send(newfd,buf,sizeof(buf),0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		return -1;
	}

	printf("2\n");

	if (strcmp(result[line+4],msg.passwd) != 0)
	{
		sprintf(buf,"您输入的密码不正确");
		printf("%s\n",buf);

		if (send(newfd,buf,sizeof(buf),0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		return -1;
	}
	
	char u_b1 = '1'; //判断身份
	char u_b2 = '0';
	
	if (msg.usertype == USER )
	{
		if (strncasecmp(result[line+1] ,&u_b1, 1) == 0)
		{
			sprintf(buf,"您是管理员，请选择管理员登陆!");
			printf("%s\n",buf);

			if (send(newfd,buf,sizeof(buf),0) < 0)
			{
				ERR_MSG("send");
				return -1;
			}

			return -1;
		}
	}
	else if (msg.usertype == ADMIN)
	{
		if (strncasecmp(result[line+1] ,&u_b2, 1) == 0)
		{
			sprintf(buf,"您是员工，请选择员工登陆!");
			printf("%s\n",buf);

			if (send(newfd,buf,sizeof(buf),0) < 0)
			{
				ERR_MSG("send");
				return -1;
			}

			return -1;
		}

	}

	printf("3\n");

	char c ='1';
	if (strncasecmp(result[line+2] ,&c, 1) == 0)
	{
		sprintf(buf,"请勿重复登陆!!");
		printf("%s\n",buf);

		if (send(newfd,buf,sizeof(buf),0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
		return -1;
	}

	sprintf(buf,"***登陆成功\n");
	printf("%s\n",buf);
	if (send(newfd,buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}
	
	printf("4\n");
	//登陆成功后将登陆状态改为上线状态
	char sql2[128] = "";
	sprintf(sql2,"update  usrinfo set state=1 where staffno = \"%s\";", result[line]);
	char* errmsg2 = NULL;
	if (sqlite3_exec(db, sql2, NULL, NULL,&errmsg2) != 0)
	{
		printf("__%d__ errmsg: %s\n",__LINE__, errmsg2);
		return -1;
	}
	printf("改为上线(state = 1)状态成功\n");

	//释放空间
    sqlite3_free_table(result);

	return 0;

}

int quit(MSG msg,sqlite3 *db,int newfd) //退出
{
	char sql1[128] ="select * from usrinfo";
	char** result = NULL;
	char* errmsg = NULL;
	int row, column;
	int fg = 0;

	char buf[128] = "";

	if (sqlite3_get_table(db, sql1, &result, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ errmsg:%s\n", __LINE__,errmsg);
		return -1;
	}

	int line = 0;

	bzero(buf,sizeof(buf));
	for(line = 12; line <= row*column; line+=12)
	{
		if (atoi(result[line]) ==  msg.id)
		{
			fg = 1;
			break;
		}
	}

	char sql2[128] = "";
	//if (fg == 1)
	{
		sprintf(sql2,"update usrinfo set state=0 where staffno = \"%s\";", result[line]);
		char* errmsg2 = NULL;
		if (sqlite3_exec(db, sql2, NULL, NULL,&errmsg2) != 0)
		{
			printf("__%d__ errmsg: %s\n",__LINE__, errmsg2);
			return -1;
		}
		printf("改为离线(state = 0)状态成功\n");
	}

	 //释放空间
    sqlite3_free_table(result);

	return 0;

}


int user_admin_query(MSG msg,sqlite3 *db,int newfd) //查询员工信息
{
	printf("进入查序系统\n");

	char sql1[128] ="select * from usrinfo";
	char** result = NULL;
	char* errmsg = NULL;
	int row, column;
	int fg = 0;

	char buf[1024] = "";

	if (sqlite3_get_table(db, sql1, &result, &row, &column, &errmsg) != 0)
	{
		printf("__%d__ errmsg:%s\n", __LINE__,errmsg);
		return -1;
	}

	int line = 0;
	int i = 0;

	bzero(buf,sizeof(buf));
	for(line = 12; line <= row*column; line+=12)
	{
		if (atoi(result[line]) ==  msg.id)
		{
			fg = 1;
			for (i = 0; i<12; i++)
			{
				sprintf(buf+strlen(buf),"%s   \t",result[line+i]);
			}
			break;
		}
	}
	if (!fg)
	{
		sprintf(buf,"***您输入的工号不正确,没有该员工***");
	}

	if(send(newfd,buf,sizeof(buf),0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}
	printf("%s\n",buf);
	 //释放空间
    sqlite3_free_table(result);

	return 0;
}


int admin_adduser(int sfd, sqlite3 *db,MSG msg) //添加新员工
{
	char sql1[1024] ="select * from usrinfo";
	char** result = NULL;
	char* errmsg = NULL;
	int row, column;

	char buf[128] = "";


	if (sqlite3_get_table(db, sql1, &result, &row, &column, &errmsg) != 0)
	{
			printf("__%d__ errmsg:%s\n", __LINE__,errmsg);
			return -1;
		
	}
	int line = 0;
	
	bzero(buf,sizeof(buf));
	for(line =12 ; line <= row*column; line+=12)
	{
		if (atoi(result[line]) == msg.info.no)
		{
			sprintf(buf,"该工号已被使用");
			printf("%s\n", buf);

			if (send(sfd,buf, sizeof(buf), 0) < 0)
			{
				ERR_MSG("send");
				return -1;
			}
			return -1;
		}
	}

	bzero(sql1, sizeof(sql1));

	sprintf (sql1, "insert into usrinfo values(%d,%d,0,\"%s\",\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,%f);", msg.info.no, \
			msg.info.usertype, msg.info.name, msg.info.passwd, msg.info.age, msg.info.phone, msg.info.addr, msg.info.work, \
			msg.info.date, msg.info.level, msg.info.salary);
	char* errmsg1 = NULL;
	if (sqlite3_exec(db, sql1, NULL, NULL, &errmsg1) != 0)
	{
		printf("__%d__ errmsg1: %s\n", __LINE__, errmsg1);
		return -1;
	}
	bzero(buf,sizeof(buf));
	sprintf(buf,"添加成功");
	if (send(sfd,buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	printf("添加成功\n");
	 //释放空间
    sqlite3_free_table(result);

	return 0;

}


int admin_deluser(int sfd,sqlite3 *db,MSG msg)//删除员工
{
	printf("进入删除员工模块\n");

	char sql1[1024] ="select * from usrinfo";
	char** result = NULL;
	char* errmsg = NULL;
	int row, column;

	char buf[128] = "";
	int  flag = 0;


	if (sqlite3_get_table(db, sql1, &result, &row, &column, &errmsg) != 0)
	{
			printf("__%d__ errmsg:%s\n", __LINE__,errmsg);
			return -1;
		
	}
	int line = 0;
	
	bzero(buf,sizeof(buf));
	for(line =12 ; line <= row*column; line+=12)
	{
		if (atoi(result[line]) == msg.id)
		{
			flag = 1;
		}
	}

	if (!flag)
	{
		sprintf(buf,"您输入的工号不正确，没有该员工!");
		if (send(sfd,buf,sizeof(buf),0))
		{
			ERR_MSG("send");
			return -1;
		}
	}

	bzero(sql1, sizeof(sql1));
	sprintf (sql1, "delete from usrinfo where staffno=%d;", msg.id);
	char* errmsg1 = NULL;

	if (sqlite3_exec(db, sql1, NULL, NULL, &errmsg1) != 0)
	{
		printf("__%d__ errmsg1: %s\n", __LINE__, errmsg1);
		return -1;
	}
	bzero(buf,sizeof(buf));
	sprintf(buf,"该员工已从系统删除");

	if (send(sfd,buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	printf("删除成功\n");
	 //释放空间
    sqlite3_free_table(result);
	return 0;


}
	
int modify(int sfd,sqlite3 *db,MSG msg) //管理员修改员工信息
{
	printf("进入员工信息修改系统\n");

	char sql1[1024] ="select * from usrinfo";
	char** result = NULL;
	char* errmsg = NULL;
	char* errmsg2 = NULL;
	int row, column;

	char buf[600] = "";
	char cha_inf[180] = ""; //记录修改信息
	int s_id = 0;
	int  flag = 0;
	char t[128] = "";
	strcpy(t,get_time());


	if (sqlite3_get_table(db, sql1, &result, &row, &column, &errmsg) != 0)
	{
			printf("__%d__ errmsg:%s\n", __LINE__,errmsg);
			return -1;
		
	}
	int line = 0;
	
	bzero(buf,sizeof(buf));
	for(line =12 ; line <= row*column; line+=12)
	{
		if (atoi(result[line]) == msg.info.no)
		{
			flag = 1;
			break;
		}
	}

	if (!flag)
	{
		sprintf(buf,"您输入的工号不正确，没有该员工!");
		if (send(sfd,buf,sizeof(buf),0) < 0)
		{
			ERR_MSG("send");
			return -1;
		}
	}

	sprintf(buf,"*** no正确***");
	printf("no正确\n");

	s_id = msg.id;
	printf("%d\n",s_id);

	if (send(sfd,buf,sizeof(buf),0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	bzero(buf,sizeof(buf));
	bzero(sql1,sizeof(sql1));

	if (recv(sfd,&msg,sizeof(msg),0) < 0)
	{
		ERR_MSG("recv");
		return -1;
	}
	printf("修改的id为 %d\n",msg.info.no);

	switch(msg.flags)
	{
	case 1:
		sprintf(sql1,"update usrinfo set usertype=%d where staffno= \"%s\";",msg.info.age,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的身份为 %d \n",s_id,msg.info.no,msg.info.age);
		break;
	case 2:
		sprintf(sql1,"update usrinfo set name=\"%s\" where staffno= \"%s\";",msg.info.addr,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的名字为 %s \n",s_id,msg.info.no,msg.info.addr);
		break;
	case 3:
		sprintf(sql1,"update usrinfo set passwd=\"%s\" where staffno= \"%s\";",msg.info.addr,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的密码为 %s \n",s_id,msg.info.no,msg.info.addr);
		break;
	case 4:
		sprintf(sql1,"update usrinfo set age=%d where staffno= \"%s\";",msg.info.age,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的年龄为 %d \n",s_id,msg.info.no,msg.info.age);
		break;
	case 5:
		sprintf(sql1,"update usrinfo set phone=\"%s\" where staffno= \"%s\";",msg.info.addr,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的电话为 %s \n",s_id,msg.info.no,msg.info.addr);
		break;
	case 6:
		sprintf(sql1,"update usrinfo set add =\"%s\" where staffno= \"%s\";",msg.info.addr,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的地址为 %s \n",s_id,msg.info.no,msg.info.addr);
		break;
	case 7:
		sprintf(sql1,"update usrinfo set work=\"%s\" where staffno= \"%s\";",msg.info.addr,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的工作为 %s \n",s_id,msg.info.no,msg.info.addr);
		break;
	case 8:
		sprintf(sql1,"update usrinfo set date=\"%s\" where staffno= \"%s\";",msg.info.addr,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的入职年月为 %s \n",s_id,msg.info.no,msg.info.addr);
		break;
	case 9:
		sprintf(sql1,"update usrinfo set level=%d where staffno= \"%s\";",msg.info.age,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的等级为 %d \n",s_id,msg.info.no,msg.info.age);
		break;
	case 10:
		sprintf(sql1,"update usrinfo set salary=%d where staffno= \"%s\";",msg.info.age,result[line]);
		sprintf(cha_inf,"%d 修改了 %d 的工资为 %d \n",s_id,msg.info.no,msg.info.age);
		break;
	case 11:
		sprintf (sql1, "update usrinfo set usertype=%d name=\"%s\" passwd=\"%s\" age=%d phone=\"%s\" addr=\"%s\" work=\"%s\" date=\"%s\" levvel=%d salary=%f where staffno= \"%s\";",\
				msg.info.usertype, msg.info.name, msg.info.passwd, msg.info.age, msg.info.phone, msg.info.addr, msg.info.work, \
				msg.info.date, msg.info.level, msg.info.salary,result[line]);

		sprintf(cha_inf,"%d 修改了 %d 的所有信息\n",s_id,msg.info.no);
		break;
	default:
		printf("标志位接收错误\n");
	}
	printf("标志位接收成功\n");

	char* errmsg1 = NULL;
	if (sqlite3_exec(db, sql1, NULL, NULL, &errmsg1) != 0)
	{
		printf("__%d__ errmsg1: %s\n", __LINE__, errmsg1);
		return -1;
	}

	bzero(buf,sizeof(buf));
	sprintf(buf,"修改成功\n");

	if (send(sfd,buf, sizeof(buf), 0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}
	printf("%s\n",buf);

	bzero(buf,sizeof(buf));
	
	
	sprintf(buf,"insert into historyinfo values (\"%s\", %d, \"%s\");",t,msg.id,cha_inf);
	printf("%s\n",buf);


	if (sqlite3_exec(db, buf, NULL, NULL, &errmsg2) != 0)
	{
		printf("__%d__ errmsg1: %s\n", __LINE__, errmsg2);
		return -1;
	}

	printf("加入历史记录表成功\n"); 

	 //释放空间
    sqlite3_free_table(result);

	return 0;

}

int admin_history(int sfd,sqlite3 *db,MSG msg)  //查看历史记录
{
	printf("进入查序历史记录系统\n");

	char sql1[1024] ="select * from historyinfo";
	char** result = NULL;
	char* errmsg = NULL;
	int row, column;

	char buf[1024] = "";
	int  flag = 0;


	if (sqlite3_get_table(db, sql1, &result, &row, &column, &errmsg) != 0)
	{
			printf("__%d__ errmsg:%s\n", __LINE__,errmsg);
			return -1;
		
	}
	int line = 0;
	
	bzero(buf,sizeof(buf));
	for(line =0; line <= row*column; line+=3)
	{
		bzero(buf,sizeof(buf));
		sprintf(buf,"%12s %8s %8s",result[line],result[line+1],result[line+2]);
		if(send(sfd,buf,sizeof(buf),0)<0)
		{
			ERR_MSG("send");
			return -1;
		}
		printf("%s\n",buf);

	}

	bzero(buf,sizeof(buf));
	sprintf(buf,"***");
	if (send(sfd,buf,sizeof(buf),0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	printf("打印历史表成功\n");

 //释放空间
    sqlite3_free_table(result);
	
	return 0;
}


char *get_time()
{
	static char buf[1024] = "";
	memset(buf, 0, sizeof(1024));
	time_t t;
	t = time(NULL);

	struct tm* info = localtime(&t);
	if(NULL == info)
	{
		perror("localtime");
	}

	sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", info->tm_year+1900, info->tm_mon+1, info->tm_mday, \
			info->tm_hour, info->tm_min, info->tm_sec);

	return buf;
}

int admin_all_inf(int sfd,sqlite3 *db,MSG msg)//查询所有员工信息
{
	printf("进入查序记录系统\n");

	char sql1[1024] ="select * from usrinfo";
	char** result = NULL;
	char* errmsg = NULL;
	int row, column;

	char buf[128] = "";
	int  flag = 0;


	if (sqlite3_get_table(db, sql1, &result, &row, &column, &errmsg) != 0)
	{
			printf("__%d__ errmsg:%s\n", __LINE__,errmsg);
			return -1;
		
	}
	int line = 0;
	
	bzero(buf,sizeof(buf));
	for(line =12; line <= (row+1)*column; line++)
	{
		sprintf(buf+strlen(buf),"%s   \t",result[line]);

		if ((line+1)%12 == 0)
		{
			if(send(sfd,buf,sizeof(buf),0)<0)
			{
				ERR_MSG("send");
				return -1;
			}
			printf("%s\n",buf);
			memset(buf,0,sizeof(buf));
			}
	
	}
	printf("所有信息已发送完毕\n");

	bzero(buf,sizeof(buf));
	sprintf(buf,"***");
	printf("%s\n",buf);
	if (send(sfd,buf,sizeof(buf),0) < 0)
	{
		ERR_MSG("send");
		return -1;
	}

	printf("发送结束\n");
	return -1;


}

