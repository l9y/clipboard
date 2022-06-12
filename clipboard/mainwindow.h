#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    void initWithApp(QApplication* app);

    void loginWeb();

    void connectUiEvents();

protected:
    Ui::MainWindow *ui;

    QApplication *app;

    QNetworkAccessManager *networkMgr = nullptr;
    QNetworkAccessManager *downloadMgr = nullptr;
    QString *token = nullptr;
    QString *ws = nullptr;
    QString *serverPath = nullptr;

private slots:
    void on_btnConnect_clicked();
    void networkFinish(QNetworkReply *response);
    void onWsConnected();
    void onWsClosed();
    void onWsTextMessageArrive(QString message);
    void onClipboardDataChanged();

private:
    QWebSocket m_webSocket;
    void connectWs();

};
#endif // MAINWINDOW_H
