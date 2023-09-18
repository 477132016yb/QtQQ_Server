#include "QtQQ_Server.h"

const int tcpPort = 6666;
const int udpPort = 8888;

QtQQ_Server::QtQQ_Server(QWidget *parent)
    : QDialog(parent)
    ,m_pixPath("")
{
    ui.setupUi(this);
    if (!connectMySql()) {
        QMessageBox::information(NULL, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("连接数据库失败"));
        close();
    }
    setMap();
    initComboxBoxData();
    ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //初始化查询公司群所有员工信息
    m_depID = getCompDepId();
    m_compDepId = getCompDepId();

    updateTableData();

    //定时刷新数据
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
    QString itemText;//组合框项的文本

    //获取公司总的部门数
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
        
        //设置员工所属部门组合框的数据为响应的部门QQ号
        ui.employeeDepBox->setItemData(i, QString(row[0]).toInt());
    }

    //多一个公司群部门
    for (int i = 0; i <= depCounts; i++) {
        itemText = ui.departmentBox->itemText(i);
        sql = QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(itemText);
        res = DBconn::getInstance()->myQuery(sql.toStdString());
        row = mysql_fetch_row(res);

        //设置部门组合框的数据为响应的部门QQ号
        ui.departmentBox->setItemData(i, QString(row[0]).toInt());
    }
}

void QtQQ_Server::setMap()
{
    m_depNameMap.insert(QString::fromLocal8Bit("2001"), QString::fromLocal8Bit("人事部"));
    m_depNameMap.insert(QString::fromLocal8Bit("2002"), QString::fromLocal8Bit("研发部"));
    m_depNameMap.insert(QString::fromLocal8Bit("2003"), QString::fromLocal8Bit("市场部"));

    m_statuMap.insert(QString::fromLocal8Bit("1"), QString::fromLocal8Bit("有效"));
    m_statuMap.insert(QString::fromLocal8Bit("0"), QString::fromLocal8Bit("已注销"));

    m_onlineMap.insert(QString::fromLocal8Bit("1"), QString::fromLocal8Bit("离线"));
    m_onlineMap.insert(QString::fromLocal8Bit("2"), QString::fromLocal8Bit("在线"));
    m_onlineMap.insert(QString::fromLocal8Bit("3"), QString::fromLocal8Bit("忙碌"));
}

void QtQQ_Server::initTcpSocket()
{
    m_tcpServer = new TcpServer(tcpPort);
    m_tcpServer->run();

    //收到tcp客户端发来的信息之后进行udp广播
    connect(m_tcpServer, &TcpServer::signalTcpMsgComes, this, &QtQQ_Server::onUDPbroadMsg);
}

void QtQQ_Server::initUdpSocket()
{
    m_udpSender = new QUdpSocket(this);
    //m_udpSender->bind(QHostAddress::AnyIPv4, udpPort);
    ////收到数据，触发readyRead
    //connect(m_udpSender, &QUdpSocket::readyRead, [this] {
    //    //没有可读的数据就返回
    //    if (!m_udpSender->hasPendingDatagrams() ||
    //        m_udpSender->pendingDatagramSize() <= 0)
    //        qDebug() << QString::fromLocal8Bit("没有可读的数据");
    //    else {
    //        QNetworkDatagram recv_datagram = m_udpSender->receiveDatagram();
    //        const QString recv_text = QString::fromUtf8(recv_datagram.data());
    //        qDebug() << QString::fromLocal8Bit("UDP读取到数据 ") << recv_text;
    //    }
    //});
}

bool QtQQ_Server::connectMySql()
{
    return  DBconn::getInstance()->getConnection("localhost", "root", "123456", "qtqq");
}

int QtQQ_Server::getCompDepId()
{
    QString sql = QString("SELECT departmentID FROM tab_department WHERE department_name = '%1'").arg(QString::fromLocal8Bit("公司群"));
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row = mysql_fetch_row(res);
    QString id = QString(row[0]);
    return id.toInt();
}

void QtQQ_Server::updateTableData(int depID, int employeeID)
{
    ui.tableWidget->clear();
    QString sql;
    if (depID&&depID!=m_compDepId) {//不是公司群
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

    //设置行列数
    ui.tableWidget->setRowCount(rows);
    ui.tableWidget->setColumnCount(cols);

    QStringList header;
    header << QString::fromLocal8Bit("部门")
        << QString::fromLocal8Bit("员工工号")
        << QString::fromLocal8Bit("员工姓名")
        << QString::fromLocal8Bit("员工签名")
        << QString::fromLocal8Bit("员工状态")
        << QString::fromLocal8Bit("员工头像")
        << QString::fromLocal8Bit("在线状态");
    ui.tableWidget->setHorizontalHeaderLabels(header);

    //设置列等宽
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

    //检测员工qq号是否输入正确
    if (!ui.queryIDLineEdit->text().length()) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("请输入员工的qq号"));
        ui.queryIDLineEdit->setFocus();
        return;
    }
    //检测员工qq合法性
    int employeeID = ui.queryIDLineEdit->text().toInt();
    QString sql = QString("SELECT * FROM tab_employees WHERE employeeID = %1").arg(employeeID);
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("请输入正确的员工的qq号"));
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

    //检测员工qq号是否输入正确
    if (!ui.logoutIDLineEdit->text().length()) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("请输入员工的qq号"));
        ui.logoutIDLineEdit->setFocus();
        return;
    }
    //检测员工qq合法性
    int employeeID = ui.logoutIDLineEdit->text().toInt();
    QString sql = QString("SELECT employee_name FROM tab_employees WHERE employeeID = %1").arg(employeeID);
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("请输入正确的员工的qq号"));
        ui.logoutIDLineEdit->setFocus();
        return;
    }
    //将员工的状态设置为0
    sql = QString("UPDATE tab_employees SET status = 0 WHERE employeeID = %1").arg(employeeID);
    DBconn::getInstance()->myQuery(sql.toStdString());
    QMessageBox::information(this, QString::fromLocal8Bit("提示"),
        QString::fromLocal8Bit("员工%1的qq:%2已被注销").arg(QString(row[0])).arg(employeeID));
}

void QtQQ_Server::on_selectPictureBtn_clicked()
{
    m_pixPath = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择头像"),
                                             ".",
                                              "*.png;;*.jpg");
    if (!m_pixPath.size()) {
        return;
    }
    //将头像显示到标签
    QPixmap pixmap(m_pixPath);

    qreal widthRatio = (qreal)ui.headLabel->width() / (qreal)pixmap.width();
    qreal heightRatio = (qreal)ui.headLabel->height() / (qreal)pixmap.height();

    QSize size(pixmap.width() * widthRatio, pixmap.height() * heightRatio);
    ui.headLabel->setPixmap(pixmap.scaled(size));
}

void QtQQ_Server::on_addBtn_clicked()
{
    //检测员工姓名输入
    QString strName = ui.nameLineEdit->text();
    if (!strName.size()) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("请输入要添加的员工姓名！"));
        ui.nameLineEdit->setFocus();
        return;
    }

    //判断是否选择头像
    if (!m_pixPath.size()) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("请选择员工头像路径！"));
        return;
    }

    //数据库插入新的员工数据
    //获取员工qq号
    QString sql = QString("SELECT MAX(employeeID) FROM tab_employees");
    MYSQL_RES* res = DBconn::getInstance()->myQuery(sql.toStdString());
    MYSQL_ROW row = mysql_fetch_row(res);
    int employeeID = QString(row[0]).toInt() + 1;

    //部门QQ号
    int depID = ui.employeeDepBox->currentData().toInt();
    //m_pixPath.replace("/", "\\\\");

    //插入
    const QString code = QString("123");
    sql = QString("INSERT INTO tab_employees(departmentID,employeeID,employee_name,picture)\VALUES(%1,%2,'%3','%4')")
                   .arg(depID).arg(employeeID).arg(strName).arg(m_pixPath);
    DBconn::getInstance()->myQuery(sql.toStdString());
    sql = QString("INSERT INTO tab_accounts(employeeID,account,code)\VALUES(%1,'%2','%3')")
        .arg(employeeID).arg(strName).arg(code);
    DBconn::getInstance()->myQuery(sql.toStdString());

    QMessageBox::information(this, QString::fromLocal8Bit("提示"),
        QString::fromLocal8Bit("新增员工成功！"));
    m_pixPath = "";
    ui.nameLineEdit->clear();
    ui.headLabel->setText(QString::fromLocal8Bit("员工寸照"));
}

void QtQQ_Server::onUDPbroadMsg(QByteArray& btData)
{
    for (quint16 port = udpPort; port < udpPort + 20; port++) {
        m_udpSender->writeDatagram(btData, btData.size(), QHostAddress::Broadcast, port);
    }
}
