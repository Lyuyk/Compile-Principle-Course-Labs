#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_openfile_clicked();

    void on_action_Quit_triggered();

    void on_pushButton_closefile_clicked();

    void on_pushButton_compress_clicked();

    void on_pushButton_decompress_clicked();

    void on_pushButton_eSaveTargetCode_clicked();

private:
    Ui::MainWindow *ui;
    QStringList srcCodeList;
    QString compressCode;
    QString decompressCode;

};
#endif // MAINWINDOW_H
