#include "TcpServer.h"
#include<QTcpSocket>

TcpServer::TcpServer(int port):m_port(port)
{}

TcpServer::~TcpServer()
{}

bool TcpServer::run()
{
	if (this->listen(QHostAddress::AnyIPv4,m_port)) {
		qDebug() << QString::fromLocal8Bit("����˼����˿� %1 �ɹ���").arg(m_port);
		return true;
	}
	else {
		qDebug() << QString::fromLocal8Bit("����˼����˿� %1 ʧ�ܣ�").arg(m_port);
		return false;
	}
	return false;
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
	qDebug() << QString::fromLocal8Bit("�µ����ӣ�") << socketDescriptor << endl;
	TcpSocket* tcpsocket = new TcpSocket();
	tcpsocket->setSocketDescriptor(socketDescriptor);
	tcpsocket->run();

	//�յ��ͻ������ݣ�server���д���
	connect(tcpsocket, SIGNAL(signalGetDataFromClient(QByteArray&, int)), this, SLOT(SocketDataProcessing(QByteArray&, int)));
	//�յ��ͻ��˶Ͽ����Ӻ�server���д���
	connect(tcpsocket, SIGNAL(signlClientDisconnect(int)), this, SLOT(SocketDisconnected(int)));

	//��socket��ӵ�������
	m_tcpSocketConnectList.append(tcpsocket);
}

void TcpServer::SocketDataProcessing(QByteArray& sendData, int descriptor)
{
	for (int i = 0; i < m_tcpSocketConnectList.length(); i++) {
		QTcpSocket* item = m_tcpSocketConnectList[i];
		if (descriptor == item->socketDescriptor()) {
			qDebug() << QString::fromLocal8Bit("����IP��") << item->peerAddress().toString()
				<< QString::fromLocal8Bit("���͵�����:") << QString(sendData);
			
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
			m_tcpSocketConnectList.removeAt(i);//���������Ƴ�
			item->deleteLater();//������Դ
			qDebug() << QString::fromLocal8Bit("TcpSocket�Ͽ�����:") << descriptor << endl;
			return;
		}
	}
}
