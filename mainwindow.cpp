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

    // ВАЖНО: Передаем объект через rootContext ДО загрузки QML
    ui->quickWidget->rootContext()->setContextProperty("mapData", mapData);

    // Подключаем сигналы MapData к слотам MainWindow
    connectMapDataSignals(mapData);
    mapData -> setSelectedRegion("11001");
    // Загружаем QML
    ui->quickWidget->setSource(QUrl("qrc:/qml/MapComponent.qml"));
    ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // Устанавливаем начальное сообщение в статус-баре
    ui->statusBar->showMessage("Кликните на регион для получения информации");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectMapDataSignals(MapData *mapData)
{
    // Обработка клика по региону
    connect(mapData, &MapData::regionClicked, this, &MainWindow::onRegionClicked);

    // Обработка изменения выбора
    connect(mapData, &MapData::selectedRegionChanged, this, &MainWindow::onSelectedRegionChanged);

    // Обработка изменения статуса региона
    connect(mapData, &MapData::regionStatusChanged, this, &MainWindow::onRegionStatusChanged);

    // Обработка загрузки регионов
    connect(mapData, &MapData::regionsChanged, this, &MainWindow::onRegionsChanged);
}

// Обработчик клика по региону - БЕЗ блокирующего диалога
void MainWindow::onRegionClicked(const QString &regionId, const QString &regionName)
{
    qDebug() << "=== Region Clicked ===";
    qDebug() << "ID:" << regionId;
    qDebug() << "Name:" << regionName;

    // Получаем полную информацию о регионе
    MapData *mapData = qobject_cast<MapData*>(sender());
    if (mapData) {
        QVariantMap region = mapData->getRegionById(regionId);

        if (!region.isEmpty()) {
            QString status = region["status"].toString();
            QString postalCode = region["postal-code"].toString();

            // Формируем красивое сообщение со статусом
            QString statusText;
            if (status == "danger") {
                statusText = "⚠️ ОПАСНОСТЬ";
            } else if (status == "warning") {
                statusText = "⚡ ПРЕДУПРЕЖДЕНИЕ";
            } else {
                statusText = "✓ В норме";
            }

            // Обновляем строку состояния с полной информацией
            QString message = QString("Выбран: %1 | ID: %2 | Статус: %3 | Почт. код: %4")
                                  .arg(regionName)
                                  .arg(regionId)
                                  .arg(statusText)
                                  .arg(postalCode.isEmpty() ? "—" : postalCode);

            ui->statusBar->showMessage(message);

            // Логируем для отладки
            qDebug() << "Отображена информация:" << message;
        }
    }
}

// Обработчик изменения выбранного региона
void MainWindow::onSelectedRegionChanged(const QString &regionId)
{
    qDebug() << "Selected region changed to:" << regionId;

    if (regionId.isEmpty()) {
        ui->statusBar->showMessage("Выбор снят. Кликните на регион для получения информации");
    }
}

// Обработчик изменения статуса региона
void MainWindow::onRegionStatusChanged(const QString &regionId, const QString &status)
{
    qDebug() << "Region" << regionId << "status changed to:" << status;

    MapData *mapData = qobject_cast<MapData*>(sender());
    if (mapData) {
        QVariantMap region = mapData->getRegionById(regionId);
        if (!region.isEmpty()) {
            QString name = region["name"].toString();
            ui->statusBar->showMessage(
                QString("Статус региона %1 изменен на: %2").arg(name).arg(status),
                3000  // Показать на 3 секунды
                );
        }
    }
}

// Обработчик загрузки регионов
void MainWindow::onRegionsChanged()
{
    MapData *mapData = qobject_cast<MapData*>(sender());
    if (mapData) {
        int count = mapData->regions().size();
        qDebug() << "Regions loaded:" << count;
        ui->statusBar->showMessage(QString("Загружено регионов: %1").arg(count), 2000);
    }
}
