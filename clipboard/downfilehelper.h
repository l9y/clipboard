#ifndef DOWNFILEHELPER_H
#define DOWNFILEHELPER_H

#include <QString>
#include <QNetworkAccessManager>
#include <QApplication>
#include <QtCore>

class DownFileHelper : public QObject
{
    Q_OBJECT

public:
    DownFileHelper(QApplication *app, QString url, QString fileName);
    ~DownFileHelper();
private slots:
    void readingReadyBytesToFile();
    void downloadFinish(QNetworkReply *reply);
private:
    QString url, fileName;
    QNetworkAccessManager *downloadMgr;
    QNetworkReply *downloadReply;
    QFile *currentFile;
    void downloadFile();
};

#endif // DOWNFILEHELPER_H
