#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // Позиционирование окна программы по центру экрана.
    QDesktopWidget desktop;
    QRect          rect   = desktop.availableGeometry(desktop.primaryScreen());
    QPoint         center = rect.center();
    QPoint         xy_main_wnd;
    xy_main_wnd.setX(center.x() - (w.width() / 2));
    xy_main_wnd.setY(center.y() - (w.height() / 2));
    w.move(xy_main_wnd);
    w.setFixedSize(300, 220);

    QString app_dir = a.applicationDirPath();

    w.show();
    return a.exec();
}
