#include "mainwindow.h"
#include "singleapplication.h"
#include "appsettings.h"

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv);
    SingleApplication::setApplicationName("Orson");
    SingleApplication::setOrganizationName("orson");
    SingleApplication::setApplicationVersion("0.0.1");

    AppSettings settings;
    MainWindow window;
    if (!settings.isStartMinimized())
        window.show();

    return SingleApplication::exec();
}
