#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRegExp>
#include <QMessageBox>
#include <QLabel>
#include <QSettings>
#include <QCloseEvent>
#include <QThread>

#include "remcypr_client.h"
#include "att_thread.h"

#define SETTINGS_FILE "rem_cypr.ini"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pbUpdate_clicked();

    void on_pbConnect_clicked();

    void on_pbDisconnect_clicked();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    QLabel         *dev_cnt;
    QLabel         *dev_bind;
    AttachThread   *att_th;
};

#endif // MAINWINDOW_H
