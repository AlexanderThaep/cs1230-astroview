#include "mainwindow.h"
#include "settings.h"

#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>
#include <iostream>

void MainWindow::initialize()
{
    RenderData renderData;
    SceneParser::parse("/Users/philadlamini/Documents/Academics/CS1230/proj5-PhilaDlamini/scenefiles/realtime/required/unit_cone_cap.json", renderData);
    Scene scene(width(), height(), renderData);
    astroRender = new AstroRender(scene);

    QHBoxLayout *container = new QHBoxLayout;
    container->addWidget(astroRender);
    this->setLayout(container);
}

void MainWindow::finish() {
    astroRender->finish();
    delete(astroRender);
}
