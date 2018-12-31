#include "mainwindow.h"
#include "singleapplication.h"

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv);
    MainWindow w;
    w.show();

    return SingleApplication::exec();
}
