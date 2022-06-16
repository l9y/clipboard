#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtWebSockets>
#include <QClipboard>
#include <QStandardPaths>
#include <QDir>
#include "downfilehelper.h"

static bool dontProcessSignal = false;
static bool wsConnected = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete networkMgr;
    delete downloadMgr;
    delete token;
    delete ws;
    delete serverPath;
}


void MainWindow::on_btnConnect_clicked()
{
    try {
        loginWeb();
    } catch (...) {
    }
}



void MainWindow::initWithApp(QApplication* app)
{

    this->app = app;

    auto lastToken = this->settings.value("token").toString();
    if (lastToken != nullptr && lastToken.size() > 0) {
        this->token = new QString(lastToken);
    }
    this->ui->label->setText(QString("Please enter the server address\nsuch as:192.168.0.2:5050"));

    connect(QApplication::clipboard(), SIGNAL(dataChanged()),this,SLOT(onClipboardDataChanged()));

    networkMgr = new QNetworkAccessManager(app);
    downloadMgr = new QNetworkAccessManager(app);

    connect(networkMgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(networkFinish(QNetworkReply*)));
}


// clipboard update
void MainWindow::onClipboardDataChanged()
{
    if (!wsConnected) {
        return;
    }
    if (dontProcessSignal) {
        return;
    }
    QClipboard * clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    auto msgId = QUuid::createUuid().toString();
    msgId = msgId.replace('{', '_');
    msgId = msgId.replace('}', '_');
    msgId = msgId.replace('-', '_');
    QVariantMap jsonMap;
    jsonMap.insert("msgType", 1);
    jsonMap.insert("id", msgId);
    jsonMap.insert("content", text);
    jsonMap.insert("autoCopy", true);
    jsonMap.insert("token", *this->token);
    QJsonDocument sendMsg = QJsonDocument::fromVariant(jsonMap);
    if (!sendMsg.isNull()) {
        m_webSocket.sendTextMessage(sendMsg.toJson());
    }
}




// network request


void MainWindow::loginWeb()
{
    auto serverUrlAndHost = ui->lineEdit->text();
    if (nullptr == serverUrlAndHost)
        return;
    serverPath = new QString(serverUrlAndHost);
    QUrl url;
    if (this->token) {
       url = QUrl("http://" + *serverPath + "?token=" + *this->token);
    } else {
       url = QUrl("http://" + *serverPath);
    }

    QNetworkRequest req;
    req.setUrl(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "Clipb0ard");

    networkMgr->get(req);

}


void MainWindow::networkFinish(QNetworkReply *response)
{
    auto allinfo = response->readAll();
    QJsonParseError err;
    QJsonDocument json_recv = QJsonDocument::fromJson(allinfo,&err);

    if(!json_recv.isNull()) {
        const QJsonObject o = json_recv.object();
        if (o.contains("token")) {
            QJsonValue value = o.value("token");
            if (value.isString()) {
                auto valStr = value.toString();
                this->token = new QString(value.toString());
                settings.setValue("token", valStr);
                settings.sync();
            }
        }
        if (o.contains("ws")) {
            QJsonValue value = o.value("ws");
            if (value.isString()) {
                this->ws = new QString("ws://" + value.toString());
            }
        }
    } else {
        // json error
        response->deleteLater();
        return;
    }
    response->deleteLater();
    this->connectWs();
}


// web socket

void MainWindow::connectWs()
{
    connect(&m_webSocket, &QWebSocket::connected, this, &MainWindow::onWsConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &MainWindow::onWsClosed);
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
                this, &MainWindow::onWsTextMessageArrive);
    m_webSocket.open(QUrl(*this->ws + "?token=" + *this->token));
}

void MainWindow::onWsConnected()
{
    wsConnected = true;
    qDebug()<<"[ws] connected";
    this->ui->btnConnect->setVisible(false);
    this->ui->lineEdit->setVisible(false);
    this->ui->label->setText(QString("Connected\nOpen Window send clipboard"));
}

void MainWindow::onWsClosed()
{
    wsConnected = false;
    qDebug()<<"[ws] closed";

    this->ui->btnConnect->setVisible(true);
    this->ui->lineEdit->setVisible(true);
    this->ui->label->setText(QString("Please enter the server address\neg:192.168.0.2:5050"));
    if (serverPath)  {
        this->ui->lineEdit->setText(*serverPath);
    }
}


void MainWindow::onWsTextMessageArrive(QString message)
{
    qDebug()<<"[ws] new message"<<message;

    QJsonParseError err;
    QJsonDocument json_recv = QJsonDocument::fromJson(message.toUtf8(),&err);
    if (!json_recv.isNull()) {
        QJsonObject o = json_recv.object();
        if (o.contains("msgType") && o.contains("content")) {
            auto msgTypeValue = o.value("msgType");
            int msgTypeInt = msgTypeValue.toInt(0);
            if (msgTypeInt == 1) {
                auto contentValue = o.value("content");
                if (contentValue.isString()) {
                    auto str = contentValue.toString();
                    dontProcessSignal = true;
                    QClipboard *clipboard = QApplication::clipboard();
                    clipboard->setText(str);
                    dontProcessSignal = false;
                }
            } else if (msgTypeInt == 2) {
                auto extra = o.value("extra");
                if (!extra.isNull() && extra.isObject()) {
                    auto extraObj = extra.toObject();
                    auto fileName = extraObj.value("fileName");
                    auto filePath = extraObj.value("filePath");
                    if (fileName.isString() && filePath.isString()) {
                        auto fileNameStr = fileName.toString();
                        auto filePathStr = filePath.toString();

                        auto imageUrl = "http://" + *serverPath + "/file/download/" + fileNameStr + "?token=" + *token + "&filepath=" + filePathStr;
                        qDebug() << imageUrl;
                        new DownFileHelper(this->app, imageUrl, fileNameStr);
                    }
                }
            }
        }
    }
}





