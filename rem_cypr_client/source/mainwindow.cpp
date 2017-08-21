#include "mainwindow.h"
#include "ui_mainwindow.h"

char ip_serv[20];
char sysfs_path[20];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Основной слой, отступы.
    ui->centralWidget->setLayout(ui->verticalLayout_3);
    ui->verticalLayout_3->setContentsMargins(5, 5, 5, 5);

    // Настройка таблицы.
    ui->twDevList->setRowCount(3);
    ui->twDevList->setColumnCount(3);
    for(int row = 0; row < ui->twDevList->rowCount(); row++)
        for(int column = 0; column < ui->twDevList->columnCount(); column++)
            ui->twDevList->setItem(row, column, new QTableWidgetItem());

    QStringList col_titles;
    col_titles << "sysfs" << "VendorID:ProductID" << "bind";
    ui->twDevList->setHorizontalHeaderLabels(col_titles);

    ui->twDevList->setColumnWidth(0, 60);
    ui->twDevList->setColumnWidth(1, 130);
    ui->twDevList->setColumnWidth(2, 60);
    ui->twDevList->setRowHeight(0, 20);
    ui->twDevList->setRowHeight(1, 20);
    ui->twDevList->setRowHeight(2, 20);

    ui->twDevList->setEditTriggers(QTableWidget::NoEditTriggers);

    // Контроль ввода IP.
    QString           ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp           ipRegex("^" + ipRange + "\\." + ipRange + "\\."
                              + ipRange  + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
    ui->leIP->setValidator(ipValidator);

    // Настройка строки состояния.
    dev_cnt  = new QLabel();
    dev_bind = new QLabel();
    ui->statusBar->addWidget(dev_cnt);
    ui->statusBar->addWidget(dev_bind);
    dev_cnt->setText("Доступно устройств: 0");
    dev_bind->setText("Подключено:");
    dev_cnt->setFixedWidth(150);
    dev_bind->setFixedWidth(150);

    // Загрузка настроек.
    QSettings settings(QApplication::applicationDirPath() + "/" +
                       SETTINGS_FILE, QSettings::IniFormat);
    settings.beginGroup("Options");
    QString ip = settings.value("ip").toString();
    settings.endGroup();
    ui->leIP->setText(ip);

    connect(this, SIGNAL(aboutToQuit()), SLOT(on_Quit()));

    att_th = new AttachThread();

    setWindowIcon(QIcon(QApplication::applicationDirPath() + "/rem_cypr.png"));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete att_th;
    delete dev_cnt;
    delete dev_bind;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    // Сохранение настроек при закрытии.
    QSettings settings(QApplication::applicationDirPath() + "/" +
                       SETTINGS_FILE, QSettings::IniFormat);
    settings.beginGroup("Options");
    settings.setValue("ip", ui->leIP->text());
    settings.endGroup();
    event->accept();
}

void MainWindow::on_pbUpdate_clicked()
{
    // Очистка таблицы.
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setText(QString(""));
            ui->twDevList->setItem(i, j, item);
        }
    }

    // Получение списка устройств.
    RESPONSE_PACK res_pack;
    if (!update_dev_list(ui->leIP->text().toLatin1(), res_pack))
    {
        for (int i = 0; i < res_pack.count; i++)
        {
            QTableWidgetItem *item1 = new QTableWidgetItem();
            item1->setText(QString((char*)res_pack.dev_list[i].path));
            item1->setTextAlignment(Qt::AlignCenter);
            ui->twDevList->setItem(i, 0, item1);

            QTableWidgetItem *item2 = new QTableWidgetItem();
            item2->setText(QString((char*)res_pack.dev_list[i].id));
            item2->setTextAlignment(Qt::AlignCenter);
            ui->twDevList->setItem(i, 1, item2);

            QTableWidgetItem *item3 = new QTableWidgetItem();
            item3->setText(QString::number(res_pack.dev_list[i].bind));
            item3->setTextAlignment(Qt::AlignCenter);
            ui->twDevList->setItem(i, 2, item3);
        }

        ui->twDevList->setCurrentCell(0, 0);

        char buf[50];
        sprintf(buf, "Доступно устройств: %d", res_pack.count);
        dev_cnt->setText(QString(buf));

        if (res_pack.count == 0)
        {
            sprintf(buf, "Подключено: %s", "");
            dev_bind->setText(QString(buf));
        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("Ошибка");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setInformativeText("Ошибка при обращении к серверу. Проверьте корректность IP.");
        msgBox.exec();
    }
}

void MainWindow::on_pbConnect_clicked()
{
    RESPONSE_PACK res_pack;
    int cr = ui->twDevList->currentRow();

    if (!bind_rem_dev(ui->leIP->text().toLatin1(), cr, res_pack))
    {
        on_pbUpdate_clicked();

        ui->twDevList->setCurrentCell(cr, 0);

        char buf[50];
        sprintf(buf, "Подключено: %s", ui->twDevList->item(cr, 0)->text().toLatin1().data());
        dev_bind->setText(QString(buf));

        strcpy(ip_serv, ui->leIP->text().toLatin1().data());
        strcpy(sysfs_path, ui->twDevList->item(cr, 0)->text().toLatin1().data());
        att_th->start();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("Ошибка");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setInformativeText("Устройство не подключено.");
        msgBox.exec();
    }
}

void MainWindow::on_pbDisconnect_clicked()
{
    RESPONSE_PACK res_pack;
    int cr = ui->twDevList->currentRow();

    if (!unbind_rem_dev(ui->leIP->text().toLatin1(), cr, res_pack))
    {
        on_pbUpdate_clicked();

        ui->twDevList->setCurrentCell(cr, 0);

        char buf[50];
        sprintf(buf, "Подключено: %s", "");
        dev_bind->setText(QString(buf));
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("Ошибка");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setInformativeText("Устройство не отключено.");
        msgBox.exec();
    }
}

