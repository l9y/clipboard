#include "downfilehelper.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QNetworkAccessManager>

DownFileHelper::DownFileHelper(QApplication *app, QString url, QString fileName)
{
    this->fileName = fileName;
    this->url = url;
    this->downloadMgr = new QNetworkAccessManager(app);
    this->downloadFile();
}

DownFileHelper::~DownFileHelper() {
    delete &fileName;
    delete &url;
    delete downloadMgr;
}

void DownFileHelper::downloadFile() {
    QNetworkRequest request;
    request.setUrl(this->url);
    QNetworkReply *reply = downloadMgr->get(request);
    downloadReply = reply;
    QObject::connect(reply, SIGNAL(readyRead()), this, SLOT(readingReadyBytesToFile()));
}


void DownFileHelper::readingReadyBytesToFile() {
    QNetworkReply* pReply = downloadReply;
    if (!pReply) {
        return;
    }

    const QString downloadsFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    auto fileName = QString(downloadsFolder + QDir::separator() + this->fileName);
    auto avatorFile = QFile(fileName);
    if(!avatorFile.open(QIODevice::Append)){
       return;
    }
    avatorFile.write(pReply->readAll());
    avatorFile.close();
}


