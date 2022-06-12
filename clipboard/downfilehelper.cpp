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

    const QString downloadsFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    auto downloadFileName = QString(downloadsFolder + QDir::separator() + this->fileName);
    this->currentFile = new QFile(downloadFileName);
    if(!currentFile->open(QIODevice::ReadWrite)){
       return;
    }
    this->downloadFile();
}

DownFileHelper::~DownFileHelper() {
    delete downloadMgr;
    delete downloadReply;
    delete currentFile;
}

void DownFileHelper::downloadFile() {
    QNetworkRequest request;
    request.setUrl(this->url);
    QNetworkReply *reply = downloadMgr->get(request);
    downloadReply = reply;
    QObject::connect(reply, SIGNAL(readyRead()), this, SLOT(readingReadyBytesToFile()));
    QObject::connect(downloadMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinish(QNetworkReply*)));
}


void DownFileHelper::readingReadyBytesToFile() {
    QNetworkReply* pReply = downloadReply;
    if (!pReply) {
        return;
    }
    currentFile->skip(currentFile->size());
    currentFile->write(pReply->readAll());
}

void DownFileHelper::downloadFinish(QNetworkReply * reply) {
    if (currentFile) {
        currentFile->close();
    }
    this->disconnect();
}


