
#include <QApplication>

#if !(defined( Q_OS_LINUX ) || defined( Q_OS_BSD4 ) || defined( Q_OS_WINDOWS ) || defined( Q_OS_MACOS ))
#   error "System not supported"
#endif

#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
