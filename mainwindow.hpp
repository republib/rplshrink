/*
 * Licence:
 * You can use and modify this file without any restriction.
 * There is no warranty.
 * You also can use the licence from http://www.wtfpl.net/.
 * The original sources can be found on https://github.com/republib.
*/


#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileDialog>
#include <QResizeEvent>
#include "converter.hpp"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public ConvertLogger
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    bool log(const QString& message);
    bool error(const QString& message){
        log("+++ " + message); return false;
    }
private slots:
    void on_buttonFileSelect_clicked();

    void on_buttonStop_clicked();

    void on_buttonConvert_clicked();
public slots:
    void on_threadStateChanged(Converter::State state, const QString& info);
private:
    Ui::MainWindow *ui;
    Converter* m_converter;
};

#endif // MAINWINDOW_HPP
