/****************************************************
 * @Copyright © 2021-2023 Lyuyk. All rights reserved.
 *
 * @FileName: main.cpp
 * @Brief: 程序入口源文件
 * @Module Function:
 *
 * @Current Version: 1.2
 * @Author: Lyuyk
 * @Modifier: Lyuyk
 * @Finished Date: 2023/4/1
 *
 * @Version History: 1.1
 *                   1.2 增加对Qt6的支持，纠正了Qt5转移到Qt6程序窗体自适应双重缩放问题
 *
 *
 ****************************************************/
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor); // Qt6中防止窗体二次缩放

    QApplication a(argc, argv);
    MainWindow w;
    w.setMinimumSize(1280, 768);
    w.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    w.show();
    return a.exec();
}
