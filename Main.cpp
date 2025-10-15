#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "MapData.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Создаем QML движок
    QQmlApplicationEngine engine;

    // Создаем backend для карты
    MapData mapData;

    // Регистрируем C++ тип в QML
    qmlRegisterType<MapData>("MapData", 1, 0, "MapData");

    // Устанавливаем контекст для QML
    engine.rootContext()->setContextProperty("mapData", &mapData);

    // Загружаем GeoJSON данные
    mapData.loadGeoJSON("data/rus_simple_highcharts.geo.json");

    // Загружаем QML
    engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

    return app.exec();
}