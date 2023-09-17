#pragma once

#include <QTcpSocket>

class TcpSocket  : public QTcpSocket
{
	Q_OBJECT

public:
	TcpSocket();
	~TcpSocket();
	void run();

signals:
	void signalGetDataFromClient(QByteArray&, int);//�ӿͻ����յ�����֮�����źŸ���server������Ҫ����
	void signlClientDisconnect(int);//����server�пͻ��˶Ͽ�����

private slots:
	void onReceiveData();//����readyRead����������
	void onClientDisconnect();//����ͻ��˶Ͽ�����
private:
	int	m_descriptor;//������������Ψһ��ʶ
};
