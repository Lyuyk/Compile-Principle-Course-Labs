#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
    QApplication a(argc, argv);
    MainWindow w;
    w.setMinimumSize(1280,768);
    w.show();
    return a.exec();
}
