#pragma once

#include <QTcpServer>
#include<QDebug>
#include"TcpSocket.h"

class TcpServer  : public QTcpServer
{
	Q_OBJECT

public:
	TcpServer(int port);
	~TcpServer();
public:
	bool run();//����
protected:
	//�ͻ������µ�����ʱ
	void incomingConnection(qintptr socketDescriptor);
signals:
	void signalTcpMsgComes(QByteArray&);
private slots:
	//��������
	void SocketDataProcessing(QByteArray&sendData,int descriptor);
	//�Ͽ����Ӵ���
	void SocketDisconnected(int descriptor);
private:
	int m_port;//�˿ں�
	QList<QTcpSocket*>m_tcpSocketConnectList;
};