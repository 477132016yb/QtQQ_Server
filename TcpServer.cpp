#include "TcpServer.h"
#include<QTcpSocket>

TcpServer::TcpServer(int port):m_port(port)
{}

TcpServer::~TcpServer()
{}

bool TcpServer::run()
{
	if (this->listen(QHostAddress::AnyIPv4,m_port)) {
		qDebug() << QString::fromLocal8Bit("服务端监听端口 %1 成功！").arg(m_port);
		return true;
	}
	else {
		qDebug() << QString::fromLocal8Bit("服务端监听端口 %1 失败！").arg(m_port);
		return false;
	}
	return false;
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
	qDebug() << QString::fromLocal8Bit("新的连接：") << socketDescriptor << endl;
	TcpSocket* tcpsocket = new TcpSocket();
	tcpsocket->setSocketDescriptor(socketDescriptor);
	tcpsocket->run();

	//收到客户端数据，server进行处理
	connect(tcpsocket, SIGNAL(signalGetDataFromClient(QByteArray&, int)), this, SLOT(SocketDataProcessing(QByteArray&, int)));
	//收到客户端断开连接后，server进行处理
	connect(tcpsocket, SIGNAL(signlClientDisconnect(int)), this, SLOT(SocketDisconnected(int)));

	//将socket添加到链表中
	m_tcpSocketConnectList.append(tcpsocket);
}

void TcpServer::SocketDataProcessing(QByteArray& sendData, int descriptor)
{
	for (int i = 0; i < m_tcpSocketConnectList.length(); i++) {
		QTcpSocket* item = m_tcpSocketConnectList[i];
		if (descriptor == item->socketDescriptor()) {
			qDebug() << QString::fromLocal8Bit("来自IP：") << item->peerAddress().toString()
				<< QString::fromLocal8Bit("发送的数据:") << QString(sendData);
			
			emit signalTcpMsgComes(sendData);
		}
	}
}

void TcpServer::SocketDisconnected(int descriptor)
{
	for (int i = 0; i < m_tcpSocketConnectList.length(); i++) {
		QTcpSocket* item = m_tcpSocketConnectList[i];
		int temp = item->socketDescriptor();
		if (descriptor == temp || temp == -1) {
			m_tcpSocketConnectList.removeAt(i);//从链表中移除
			item->deleteLater();//回收资源
			qDebug() << QString::fromLocal8Bit("TcpSocket断开连接:") << descriptor << endl;
			return;
		}
	}
}
