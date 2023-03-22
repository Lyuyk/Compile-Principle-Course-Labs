/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: mian.cpp
 * @Brief:
 * @Module Function:
 *
 * @Current Version: 1.2
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/3/3
 *
 * @Version History: 1.1
 *                   1.2 current version
 *
 ****************************************************/
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);//Qt5.15以上需要该函数设定防止窗体以缩放两次显示过大
    QApplication a(argc, argv);
    MainWindow w;
    w.setMinimumSize(1024,768);
    w.show();
    return a.exec();
}
