#pragma once
#include<iostream>
#include<mysql.h>
#include<string>
#include<QVector>
#include<exception>
#include<QDebug>
using namespace std;
class DBconn
{
public:
	DBconn();
	~DBconn();
	bool getConnection(string host, string userName, string passWord, string dbName);
	bool select(string op);
	bool Query(string op);
	void close();
	static DBconn* getInstance();
	MYSQL_RES* myQuery(string op);
private:
	bool closed;
	MYSQL mysql;
};