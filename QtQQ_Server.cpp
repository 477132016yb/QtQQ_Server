#include "QtQQ_Server.h"

const int tcpPort = 6666;
const int udpPort = 8888;

QtQQ_Server::QtQQ_Server(QWidget *parent)
    : QDialog(parent)
    ,m_pixPath("")
{
    ui.setupUi(this);
    if (!connectMySql()) {
        QMessageBox::information(NULL, QString::fromLocal8Bit("��ʾ"),
            QString::fromLocal8Bit("�������ݿ�ʧ��"));
        close();
    }
    setMap();
    initComboxBoxData();
    ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //��ʼ����ѯ��˾Ⱥ����Ա����Ϣ
    m_depID = getCompDepId();
    m_compDepId = getCompDepId();

    updateTableData();

    //��ʱˢ������
    m_timer = new QTimer(this);
    m_timer->setInterval(200);
    m_timer->start();
    connect(m_timer, &QTimer::timeout, this, &QtQQ_Server::onRefresh);

    initTcpSocket();
    initUdpSocket();
}

QtQQ_Server::~QtQQ_Server()
{}

void QtQQ_Server::initComboxBoxData()
{
    QString itemText;//��Ͽ�����ı�

    //��ȡ��˾�ܵĲ�����
    QString sql = QString("SELECT * FROM tab_department");
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row;
    int rows = mysql_num_rows(res);
    int depCounts = rows - 1;
    for (int i = 0; i < depCounts; i++) {
        itemText = ui.employeeDepBox->itemText(i);
        sql = QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(itemText);
        res = DBconn::getInstance()->myQuery(sql.toStdString());
        row = mysql_fetch_row(res);
        
        //����Ա������������Ͽ������Ϊ��Ӧ�Ĳ���QQ��
        ui.employeeDepBox->setItemData(i, QString(row[0]).toInt());
    }

    //��һ����˾Ⱥ����
    for (int i = 0; i <= depCounts; i++) {
        itemText = ui.departmentBox->itemText(i);
        sql = QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(itemText);
        res = DBconn::getInstance()->myQuery(sql.toStdString());
        row = mysql_fetch_row(res);

        //���ò�����Ͽ������Ϊ��Ӧ�Ĳ���QQ��
        ui.departmentBox->setItemData(i, QString(row[0]).toInt());
    }
}

void QtQQ_Server::setMap()
{
    m_depNameMap.insert(QString::fromLocal8Bit("2001"), QString::fromLocal8Bit("���²�"));
    m_depNameMap.insert(QString::fromLocal8Bit("2002"), QString::fromLocal8Bit("�з���"));
    m_depNameMap.insert(QString::fromLocal8Bit("2003"), QString::fromLocal8Bit("�г���"));

    m_statuMap.insert(QString::fromLocal8Bit("1"), QString::fromLocal8Bit("��Ч"));
    m_statuMap.insert(QString::fromLocal8Bit("0"), QString::fromLocal8Bit("��ע��"));

    m_onlineMap.insert(QString::fromLocal8Bit("1"), QString::fromLocal8Bit("����"));
    m_onlineMap.insert(QString::fromLocal8Bit("2"), QString::fromLocal8Bit("����"));
    m_onlineMap.insert(QString::fromLocal8Bit("3"), QString::fromLocal8Bit("æµ"));
}

void QtQQ_Server::initTcpSocket()
{
    m_tcpServer = new TcpServer(tcpPort);
    m_tcpServer->run();

    //�յ�tcp�ͻ��˷�������Ϣ֮�����udp�㲥
    connect(m_tcpServer, &TcpServer::signalTcpMsgComes, this, &QtQQ_Server::onUDPbroadMsg);
}

void QtQQ_Server::initUdpSocket()
{
    m_udpSender = new QUdpSocket(this);
    //m_udpSender->bind(QHostAddress::AnyIPv4, udpPort);
    ////�յ����ݣ�����readyRead
    //connect(m_udpSender, &QUdpSocket::readyRead, [this] {
    //    //û�пɶ������ݾͷ���
    //    if (!m_udpSender->hasPendingDatagrams() ||
    //        m_udpSender->pendingDatagramSize() <= 0)
    //        qDebug() << QString::fromLocal8Bit("û�пɶ�������");
    //    else {
    //        QNetworkDatagram recv_datagram = m_udpSender->receiveDatagram();
    //        const QString recv_text = QString::fromUtf8(recv_datagram.data());
    //        qDebug() << QString::fromLocal8Bit("UDP��ȡ������ ") << recv_text;
    //    }
    //});
}

bool QtQQ_Server::connectMySql()
{
    return  DBconn::getInstance()->getConnection("localhost", "root", "123456", "qtqq");
}

int QtQQ_Server::getCompDepId()
{
    QString sql = QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(QString::fromLocal8Bit("��˾Ⱥ"));
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row = mysql_fetch_row(res);
    QString id = QString(row[0]);
    return id.toInt();
}

void QtQQ_Server::updateTableData(int depID, int employeeID)
{
    ui.tableWidget->clear();
    QString sql;
    if (depID&&depID!=m_compDepId) {//���ǹ�˾Ⱥ
        sql = QString("SELECT * FROM tab_employees WHERE departmentID =%1").arg(depID);
    }
    else if (employeeID) {
        sql = QString("SELECT * FROM tab_employees WHERE employeeID =%1").arg(employeeID);
    }
    else{
        sql = QString("SELECT * FROM tab_employees");
    }
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row;

    int rows= mysql_num_rows(res);
    int cols= mysql_num_fields(res);

    //����������
    ui.tableWidget->setRowCount(rows);
    ui.tableWidget->setColumnCount(cols);

    QStringList header;
    header << QString::fromLocal8Bit("����")
        << QString::fromLocal8Bit("Ա������")
        << QString::fromLocal8Bit("Ա������")
        << QString::fromLocal8Bit("Ա��ǩ��")
        << QString::fromLocal8Bit("Ա��״̬")
        << QString::fromLocal8Bit("Ա��ͷ��")
        << QString::fromLocal8Bit("����״̬");
    ui.tableWidget->setHorizontalHeaderLabels(header);

    //�����еȿ�
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList recordList;
    for (int i = 0; i < cols; i++) {
        recordList.append(QString(mysql_fetch_field(res)->name));
    }

    for (int i = 0; i < rows; i++){
        row = mysql_fetch_row(res);
        for (int j = 0; j < cols; j++) {
            QString strData = QString(row[j]);

            QString record = recordList[j];
            if (record == QString::fromLocal8Bit("deparmentID")) {
                strData = m_depNameMap[strData];
            }
            else if (record == QString::fromLocal8Bit("status")) {
                strData = m_statuMap[strData];
            }
            else if (record == QString::fromLocal8Bit("online")) {
                strData = m_onlineMap[strData];
            }

            ui.tableWidget->setItem(i, j, new QTableWidgetItem(strData));
        }
    }
}

void QtQQ_Server::onRefresh()
{
    updateTableData(m_depID, m_employeeID);
}

void QtQQ_Server::on_queryDepartmentBtn_clicked()
{
    ui.queryIDLineEdit->clear();
    ui.logoutIDLineEdit->clear();
    m_employeeID = 0;
    m_depID = ui.departmentBox->currentData().toInt();
    updateTableData(m_depID);
}

void QtQQ_Server::on_queryIDBtn_clicked()
{
    ui.departmentBox->setCurrentIndex(0);
    ui.logoutIDLineEdit->clear();
    m_depID = m_compDepId;

    //���Ա��qq���Ƿ�������ȷ
    if (!ui.queryIDLineEdit->text().length()) {
        QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
            QString::fromLocal8Bit("������Ա����qq��"));
        ui.queryIDLineEdit->setFocus();
        return;
    }
    //���Ա��qq�Ϸ���
    int employeeID = ui.queryIDLineEdit->text().toInt();
    QString sql = QString("SELECT * FROM tab_employees WHERE employeeID = %1").arg(employeeID);
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
            QString::fromLocal8Bit("��������ȷ��Ա����qq��"));
        ui.queryIDLineEdit->setFocus();
        return;
    }
    m_employeeID = employeeID;
    updateTableData(m_depID, m_employeeID);
}

void QtQQ_Server::on_logoutBtn_clicked()
{
    ui.queryIDLineEdit->clear();
    ui.departmentBox->setCurrentIndex(0);

    //���Ա��qq���Ƿ�������ȷ
    if (!ui.logoutIDLineEdit->text().length()) {
        QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
            QString::fromLocal8Bit("������Ա����qq��"));
        ui.logoutIDLineEdit->setFocus();
        return;
    }
    //���Ա��qq�Ϸ���
    int employeeID = ui.logoutIDLineEdit->text().toInt();
    QString sql = QString("SELECT employee_name FROM tab_employees WHERE employeeID = %1").arg(employeeID);
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
            QString::fromLocal8Bit("��������ȷ��Ա����qq��"));
        ui.logoutIDLineEdit->setFocus();
        return;
    }
    //��Ա����״̬����Ϊ0
    sql = QString("UPDATE tab_employees SET status = 0 WHERE employeeID = %1").arg(employeeID);
    DBconn::getInstance()->myQuery(sql.toStdString());
    QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
        QString::fromLocal8Bit("Ա��%1��qq:%2�ѱ�ע��").arg(QString(row[0])).arg(employeeID));
}

void QtQQ_Server::on_selectPictureBtn_clicked()
{
    m_pixPath = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("ѡ��ͷ��"),
                                             ".",
                                              "*.png;;*.jpg");
    if (!m_pixPath.size()) {
        return;
    }
    //��ͷ����ʾ����ǩ
    QPixmap pixmap(m_pixPath);

    qreal widthRatio = (qreal)ui.headLabel->width() / (qreal)pixmap.width();
    qreal heightRatio = (qreal)ui.headLabel->height() / (qreal)pixmap.height();

    QSize size(pixmap.width() * widthRatio, pixmap.height() * heightRatio);
    ui.headLabel->setPixmap(pixmap.scaled(size));
}

void QtQQ_Server::on_addBtn_clicked()
{
    //���Ա����������
    QString strName = ui.nameLineEdit->text();
    if (!strName.size()) {
        QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
            QString::fromLocal8Bit("������Ҫ��ӵ�Ա��������"));
        ui.nameLineEdit->setFocus();
        return;
    }

    //�ж��Ƿ�ѡ��ͷ��
    if (!m_pixPath.size()) {
        QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
            QString::fromLocal8Bit("��ѡ��Ա��ͷ��·����"));
        return;
    }

    //���ݿ�����µ�Ա������
    //��ȡԱ��qq��
    QString sql = QString("SELECT MAX(employeeID) FROM tab_employees");
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row = mysql_fetch_row(res);
    int employeeID = QString(row[0]).toInt() + 1;

    //����QQ��
    int depID = ui.employeeDepBox->currentData().toInt();
    //m_pixPath.replace("/", "\\\\");

    //����
    const QString code = QString("123");
    sql = QString("INSERT INTO tab_employees(departmentID,employeeID,employee_name,picture)\VALUES(%1,%2,'%3','%4')")
                   .arg(depID).arg(employeeID).arg(strName).arg(m_pixPath);
    DBconn::getInstance()->myQuery(sql.toStdString());
    sql = QString("INSERT INTO tab_accounts(employeeID,account,code)\VALUES(%1,'%2','%3')")
        .arg(employeeID).arg(strName).arg(code);
    DBconn::getInstance()->myQuery(sql.toStdString());

    QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
        QString::fromLocal8Bit("����Ա���ɹ���"));
    m_pixPath = "";
    ui.nameLineEdit->clear();
    ui.headLabel->setText(QString::fromLocal8Bit("Ա������"));
}

void QtQQ_Server::onUDPbroadMsg(QByteArray& btData)
{
    for (quint16 port = udpPort; port < udpPort + 20; port++) {
        m_udpSender->writeDatagram(btData, btData.size(), QHostAddress::Broadcast, port);
    }
}
