#include "mainwindow.h"

#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>
#include <iostream>

void MainWindow::initialize(const QString &sceneFilePath)
{
    RenderData renderData;
    SceneParser::parse(sceneFilePath.toStdString(), renderData); // use the passed path
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
