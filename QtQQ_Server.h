#pragma once

#include <QtWidgets/QDialog>
#include "ui_QtQQ_Server.h"
#include"TcpServer.h"
#include"DBconn.h"
#include<QMessageBox>
#include<QHeaderView>
#include<QTableWidgetItem>
#include<QTimer>
#include<QFileDialog>
#include<QUdpSocket>
#include<QNetworkDatagram>

class QtQQ_Server : public QDialog
{
    Q_OBJECT

public:
    QtQQ_Server(QWidget *parent = nullptr);
    ~QtQQ_Server();
private:
    void initComboxBoxData();//��ʼ����Ͽ������
    void setMap();
    void initTcpSocket();
    void initUdpSocket();
    bool connectMySql();
    int getCompDepId();
    void updateTableData(int depID = 0, int employeeID = 0);
private slots:
    void onUDPbroadMsg(QByteArray&btData);
    void onRefresh();
    void on_queryDepartmentBtn_clicked();//����ź���ۺ����Զ�����
    void on_queryIDBtn_clicked();
    void on_logoutBtn_clicked();
    void on_selectPictureBtn_clicked();
    void on_addBtn_clicked();
private:
    Ui::QtQQ_ServerClass ui;

    QTimer* m_timer;
    TcpServer* m_tcpServer;//tcp�����
    QUdpSocket* m_udpSender;//udp�㲥

    int m_compDepId;
    int m_depID;//����id
    int m_employeeID;//Ա��id
    QString m_pixPath;
    QMap<QString, QString>m_statuMap;
    QMap<QString, QString>m_depNameMap;
    QMap<QString, QString>m_onlineMap;
};
