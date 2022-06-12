#ifndef DOWNFILEHELPER_H
#define DOWNFILEHELPER_H

#include <QString>
#include <QNetworkAccessManager>
#include <QApplication>

class DownFileHelper : public QObject
{
    Q_OBJECT

public:
    DownFileHelper(QApplication *app, QString url, QString fileName);
    ~DownFileHelper();
private slots:
    void readingReadyBytesToFile();
private:
    QString url, fileName;
    QNetworkAccessManager *downloadMgr;
    QNetworkReply *downloadReply;
    void downloadFile();
};

#endif // DOWNFILEHELPER_H
