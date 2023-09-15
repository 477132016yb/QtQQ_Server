#include "DBconn.h"
Q_GLOBAL_STATIC(DBconn, theInstance)
DBconn::DBconn()
{
	closed = false;
}

DBconn::~DBconn()
{
	if (!closed)
	{
		try
		{
			mysql_close(&mysql);
		}
		catch (exception&e)
		{
			qDebug() << e.what() << endl;
		}
	}
}

bool DBconn::getConnection(string host, string userName, string passWord, string dbName)
{
	mysql_init(&mysql);// 初始化一个MYSQL 连接的实例对象
//连接MySql
	if (!mysql_real_connect(&mysql, host.c_str(), userName.c_str(), passWord.c_str(), dbName.c_str(), 3306, 0, 0))
	{
		qDebug() << "Error connecting to database:" << mysql_error(&this->mysql) << endl;
		return false;
	}
	//我在win7和win10都会出现中文乱码，修改数据库编码方式解决问题
	//mysql_query(&mysql, "SET NAMES GB2312");//修改mysql编码方式
	//mysql_query(&mysql, "SET NAMES UTF8");
	return true;
}

bool DBconn::select(string op)
{
	if (mysql_query(&mysql, op.c_str()))
	{
		qDebug() << "Query failed :" << mysql_error(&this->mysql) << endl;
		return false;
	}
	MYSQL_ROW row; //char** 二维数组，存放一条条记录
	MYSQL_RES* res = mysql_store_result(&mysql);//返回结果集
	if (!res)
	{
		qDebug() << "Couldn't get result from" << mysql_error(&this->mysql) << endl;
		return false;
	}
	int count1 = mysql_num_rows(res);//获取记录个数(参数为结果集行数,仅select可用)
	int count2 = mysql_num_fields(res);//返回字段个数(表的列数)
	int k = 1;
	qDebug() << "共：" << count1 << "条记录" << endl;
	QVector<string>str_field;               //定义一个字符串数组存储字段信息  
	for (int i = 0; i < count2; i++)            //在已知字段数量的情况下获取字段名  
	{
		str_field.push_back(mysql_fetch_field(res)->name);
		//qDebug() << "\t" << str_field[i];
	}
	qDebug() << endl;
	while ((row = mysql_fetch_row(res)) != NULL) {//获取结果集(res)中的一行,并且指针指向下一行
		qDebug() << "第" << k << "行：";
		for (int i = 0; i < count2; ++i)
		{
			//qDebug() << string(row[i]) << "\t";    //输出字段值
		}
		qDebug() << endl;
		++k;
	}
	return true;
}

bool DBconn::Query(string op)
{
	int count = 0;
	if (mysql_query(&mysql, op.c_str()))
	{
		qDebug() << "Query failed :" << mysql_error(&this->mysql) << endl;
		return false;
	}
	count = mysql_affected_rows(&mysql);//参数为mysql结构体(返回操作影响行数)
	qDebug() << "更新：" << count << " 条记录" << endl;
	return true;
}

void DBconn::close()
{
	mysql_close(&mysql);
	closed = true;
}

DBconn* DBconn::getInstance()
{
	return theInstance;
}

MYSQL_RES* DBconn::myQuery(string op)
{
	if (mysql_query(&mysql, op.c_str()))
	{
		qDebug() << "Query failed :" << mysql_error(&this->mysql) << endl;
		return NULL;
	}
	return mysql_store_result(&mysql);
}
