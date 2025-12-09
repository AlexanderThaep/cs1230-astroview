#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("AstroView");
    QCoreApplication::setOrganizationName("CS 1230");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QSurfaceFormat fmt;
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow w;

    QString sceneFile = "/Users/philadlamini/Documents/Academics/CS1230/proj5-PhilaDlamini/scenefiles/realtime/required/unit_cone_cap.json"; // default path
    if (argc > 1) {
        sceneFile = argv[1]; // use command line argument if provided
    }

    w.initialize(sceneFile);
    w.resize(800, 600);
    w.show();

    int return_val = a.exec();
    w.finish();

    return return_val;
}
