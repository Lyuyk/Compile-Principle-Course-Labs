#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QObject>
#include <QTranslator>


int main(int argc, char *argv[])
{
    //QGuiApplication::setAttribute(Qt::AA_Use96Dpi);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);


    QApplication a(argc, argv);

//    QTranslator translator;
//    const QStringList uiLanguages = QLocale::system().uiLanguages();
//    for (const QString &locale : uiLanguages) {
//        const QString baseName = "eTINY_" + QLocale(locale).name();
//        if (translator.load(":/i18n/" + baseName)) {
//            a.installTranslator(&translator);
//            break;
//        }
//    }
//    if (QT_VERSION >= QT_VERSION_CHECK(5,15,2))


    MainWindow w;
    w.setMinimumSize(1280,720);
    w.show();
    return a.exec();
}
