#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QList>

namespace Ui {
    class MainWindow;
}

class DataReport;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void loadDirectory();
    void generateCSVReport();

private:
    Ui::MainWindow *ui;
    QList<DataReport *>* listData;
    QString workDir;


    void loadData();
    bool readReport(QString file);
    void showData();
    void clearData();
};

#endif // MAINWINDOW_H
