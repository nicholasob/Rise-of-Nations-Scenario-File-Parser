#include "main_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    QApplication::setApplicationName("RoN Scenario Viewer");
    QApplication::setOrganizationName("RoNScenarioViewer");
    QApplication::setApplicationVersion("1.0.0");

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
