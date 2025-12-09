#include "mainwindow.h"

#include <QHBoxLayout>

MainWindow::MainWindow()
{
    RenderData renderData;
    SceneParser::parse("/Users/philadlamini/Documents/Academics/CS1230/proj5-PhilaDlamini/scenefiles/realtime/required/directional_light_1.json", renderData);
    Scene scene(width(), height(), renderData);
    astroRender = new AstroRender(scene);

    QHBoxLayout *container = new QHBoxLayout;
    container->addWidget(astroRender);
    this->setLayout(container);
}

MainWindow::~MainWindow() {}
