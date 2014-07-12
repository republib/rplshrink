/*
 * Licence:
 * You can use and modify this file without any restriction.
 * There is no warranty.
 * You also can use the licence from http://www.wtfpl.net/.
 * The original sources can be found on https://github.com/republib.
*/


#include "mainwindow.hpp"
#include <QApplication>
char** g_argv;
int main(int argc, char *argv[])
{
    g_argv = argv;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
