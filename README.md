# Staffing-System



员工管理系统项目需求

实现目标：员工管理系统，处理对象为员工的信息。分为2个端（管理员，普通员工）登录，
一、基本功能要求：
          （1）linux系统编写；
          （2）采用Client/Server架构
		  （3）所有员工信息使用数据库或者文件存储
		  （4）登录需要账号密码，错误次数限制等。
          （5）管理员具有最高权限，包括增加员工信息，修改，删除，查看所有员工信息，考勤记录。
		  （6）普通员工仅可以查看，修改自己信息，考勤记录。
		  （7）普通员工和管理员界面分开。
		  （8）记录普通员工上班和下线时间 （*）
		  （9）有一个主界面，供选择是否为管理员登录。
		  （10）退出系统
二、考核内容

【网路编程】

设计client和Server的通讯协议，并实现Client端向Server端的消息发送

【文件 I/O编程】

【设计考勤记录的文件格式】（*）
设计注册用户的相关信息的“数据库”文件

【多线程或者进程编程】

server端需要采用TCP/UDP多线程或者selcet,epoll，用户登录之后，开启一个线程用于处理客户端的指令。

注：*为可选择项
