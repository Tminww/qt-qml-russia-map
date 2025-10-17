#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mapdata.h"
#include <QDebug>
#include <QVariant>
#include <QQmlContext>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Регистрируем C++ тип в QML
    qmlRegisterType<MapData>("MapData", 1, 0, "MapData");

    // Создаем объект MapData
    MapData *mapData = new MapData(this);

    // Загружаем GeoJSON данные
    mapData->loadGeoJSON("rus_simple_highcharts.geo.json");

    // ВАЖНО: Передаем объект через rootContext
    ui->quickWidget->rootContext()->setContextProperty("mapData", mapData);

    // Загружаем QML
    ui->quickWidget->setSource(QUrl("qrc:/qml/MapComponent.qml"));
    ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
}

MainWindow::~MainWindow()
{
    delete ui;
}
