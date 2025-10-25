# Руководство по интеграции MapComponent в проект

## Быстрый старт

### 1. Добавьте файлы в проект

Скопируйте в ваш проект:

```
your-project/
├── mapdata.h
├── mapdata.cpp
├── qml/
│   └── MapComponent.qml
└── data/
    └── your_map_data.geo.json
```

### 2. Обновите .pro файл

```qmake
QT += core gui qml quick quickwidgets widgets

SOURCES += \
    mapdata.cpp \
    # ... ваши файлы

HEADERS += \
    mapdata.h \
    # ... ваши файлы

RESOURCES += \
    resources.qrc
```

### 3. Создайте resources.qrc

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource>
        <file>qml/MapComponent.qml</file>
        <file>data/your_map_data.geo.json</file>
    </qresource>
</RCC>
```

## Интеграция в QWidgets приложение

### Вариант 1: Через Qt Designer

1. Добавьте `QQuickWidget` на форму

### Вариант 2: Программно

```cpp
#include <QQuickWidget>
#include <QQmlContext>

// В конструкторе вашего виджета
QQuickWidget *quickWidget = new QQuickWidget(this);
quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
layout()->addWidget(quickWidget);
```

### Инициализация MapData

```cpp
#include "mapdata.h"
#include <QtQml>

// 1. Регистрируем тип для QML
qmlRegisterType<MapData>("MapData", 1, 0, "MapData");

// 2. Создаем экземпляр
MapData *mapData = new MapData(this);

// 3. Загружаем данные
mapData->loadGeoJSON("your_map_data.geo.json");

// 4. Передаем в QML контекст
quickWidget->rootContext()->setContextProperty("mapData", mapData);

// 5. Загружаем QML
quickWidget->setSource(QUrl("qrc:/qml/MapComponent.qml"));
```

## Обработка событий в C++

### Подключение к сигналам

```cpp
class YourWidget : public QWidget
{
    Q_OBJECT

public:
    YourWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        // ... инициализация mapData ...

        // Подключаем сигналы
        connect(mapData, &MapData::regionClicked,
                this, &YourWidget::handleRegionClick);

        connect(mapData, &MapData::selectedRegionChanged,
                this, &YourWidget::handleSelectionChange);

        connect(mapData, &MapData::regionStatusChanged,
                this, &YourWidget::handleStatusChange);
    }

private slots:
    void handleRegionClick(const QString &regionId, const QString &regionName)
    {
        qDebug() << "User clicked:" << regionName << "ID:" << regionId;

        // Получить полную информацию о регионе
        QVariantMap region = mapData->getRegionById(regionId);
        QString status = region["status"].toString();

        // Ваша логика обработки
        if (status == "danger") {
            showWarningDialog(regionName);
        }
    }

    void handleSelectionChange(const QString &regionId)
    {
        if (regionId.isEmpty()) {
            qDebug() << "Selection cleared";
        } else {
            qDebug() << "Selected:" << regionId;
        }
    }

    void handleStatusChange(const QString &regionId, const QString &status)
    {
        qDebug() << "Region" << regionId << "status changed to:" << status;
    }

private:
    MapData *mapData;
};
```

## API MapData

### Свойства (доступны из QML и C++)

```cpp
// Получить все регионы
QVariantList regions = mapData->regions();

// Получить/установить выбранный регион
QString selected = mapData->selectedRegion();
mapData->setSelectedRegion("10312");
```

### Методы

```cpp
// Загрузить данные карты
mapData->loadGeoJSON("map_data.geo.json");

// Получить информацию о конкретном регионе
QVariantMap region = mapData->getRegionById("10312");
QString name = region["name"].toString();
QString status = region["status"].toString();
QVariantList paths = region["paths"].toList();

// Обновить статус региона
mapData->updateRegionStatus("10312", "warning");
mapData->updateRegionStatus("10202", "danger");

// Снять выбор
mapData->clearSelection();
```

### Сигналы

```cpp
// Испускается при клике на регион
void regionClicked(const QString &regionId, const QString &regionName);

// Испускается при изменении выбранного региона
void selectedRegionChanged(const QString &regionId);

// Испускается при изменении статуса региона
void regionStatusChanged(const QString &regionId, const QString &status);

// Испускается при загрузке/обновлении данных
void regionsChanged();
```

## Примеры использования

### Загрузка статусов из базы данных

```cpp
void loadRegionStatusesFromDB(MapData *mapData)
{
    QSqlQuery query("SELECT region_id, status FROM regions");

    while (query.next()) {
        QString regionId = query.value(0).toString();
        QString status = query.value(1).toString();

        mapData->updateRegionStatus(regionId, status);
    }
}
```

### Обновление статуса при получении данных с сервера

```cpp
void onDataReceived(const QJsonObject &data)
{
    QString regionId = data["region_id"].toString();
    int alertLevel = data["alert_level"].toInt();

    QString status;
    if (alertLevel >= 3) {
        status = "danger";
    } else if (alertLevel >= 2) {
        status = "warning";
    } else {
        status = "default";
    }

    mapData->updateRegionStatus(regionId, status);
}
```

### Программный выбор региона

```cpp
// Выбрать регион программно
void selectRegionFromList(const QString &regionId)
{
    mapData->setSelectedRegion(regionId);
    // Карта автоматически обновится и выделит регион
}

// Снять выбор
void clearRegionSelection()
{
    mapData->clearSelection();
}
```

### Интеграция с QTableWidget

```cpp
void setupRegionTable(QTableWidget *table, MapData *mapData)
{
    // Заполняем таблицу регионами
    QVariantList regions = mapData->regions();
    table->setRowCount(regions.size());

    for (int i = 0; i < regions.size(); ++i) {
        QVariantMap region = regions[i].toMap();

        table->setItem(i, 0, new QTableWidgetItem(region["name"].toString()));
        table->setItem(i, 1, new QTableWidgetItem(region["id"].toString()));
        table->setItem(i, 2, new QTableWidgetItem(region["status"].toString()));
    }

    // При клике в таблице выбираем регион на карте
    connect(table, &QTableWidget::cellClicked, [mapData, table](int row, int col) {
        QString regionId = table->item(row, 1)->text();
        mapData->setSelectedRegion(regionId);
    });

    // При клике на карте выделяем строку в таблице
    connect(mapData, &MapData::selectedRegionChanged, [table](const QString &regionId) {
        for (int i = 0; i < table->rowCount(); ++i) {
            if (table->item(i, 1)->text() == regionId) {
                table->selectRow(i);
                break;
            }
        }
    });
}
```

### Фильтрация регионов по статусу

```cpp
void filterRegionsByStatus(QListWidget *list, MapData *mapData, const QString &statusFilter)
{
    list->clear();

    QVariantList regions = mapData->regions();
    for (const QVariant &var : regions) {
        QVariantMap region = var.toMap();
        QString status = region["status"].toString();

        if (statusFilter.isEmpty() || status == statusFilter) {
            QString name = region["name"].toString();
            QString id = region["id"].toString();

            QListWidgetItem *item = new QListWidgetItem(
                QString("%1 (%2)").arg(name).arg(status)
            );
            item->setData(Qt::UserRole, id);
            list->addItem(item);
        }
    }
}

// Использование:
// Показать только регионы с предупреждением
filterRegionsByStatus(ui->regionList, mapData, "warning");

// Показать все
filterRegionsByStatus(ui->regionList, mapData, "");
```

### Экспорт выбранного региона

```cpp
void exportSelectedRegion(MapData *mapData)
{
    QString regionId = mapData->selectedRegion();
    if (regionId.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Регион не выбран");
        return;
    }

    QVariantMap region = mapData->getRegionById(regionId);

    QJsonObject json;
    json["id"] = region["id"].toString();
    json["name"] = region["name"].toString();
    json["status"] = region["status"].toString();
    json["postal-code"] = region["postal-code"].toString();

    QJsonDocument doc(json);

    QString fileName = QFileDialog::getSaveFileName(
        this, "Сохранить регион", "", "JSON (*.json)"
    );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
        }
    }
}
```

## Кастомизация внешнего вида

### В QML

```qml
MapComponent {
    id: mapComponent

    // Цвета фона
    backgroundColor: "#2c3e50"

    // Цвета регионов
    defaultColor: "#3498db"
    warningColor: "#f39c12"
    dangerColor: "#e74c3c"

    // Цвета выбранных регионов
    defaultActiveColor: "#2980b9"
    warningActiveColor: "#d68910"
    dangerActiveColor: "#c0392b"

    // Обводка
    strokeColor: "#ffffff"
    activeStrokeColor: "#000000"
    strokeWidth: 1
    activeStrokeWidth: 2
}
```

### Программно через QML свойства

```cpp
QObject *rootObject = quickWidget->rootObject();
if (rootObject) {
    // Изменить цвет для опасных регионов
    rootObject->setProperty("dangerColor", QColor("#ff0000"));

    // Изменить толщину обводки
    rootObject->setProperty("strokeWidth", 2.0);
}
```

## Обработка ошибок

```cpp
// Проверка загрузки QML
if (quickWidget->status() == QQuickWidget::Error) {
    qCritical() << "Ошибка загрузки QML:";
    for (const QQmlError &error : quickWidget->errors()) {
        qCritical() << error.toString();
    }
    QMessageBox::critical(this, "Ошибка", "Не удалось загрузить карту");
    return;
}

// Проверка наличия данных
if (mapData->regions().isEmpty()) {
    qWarning() << "Нет данных для отображения";
    QMessageBox::warning(this, "Предупреждение", "Карта не содержит данных");
}
```

## Требования

- Qt 5.11 или выше
- Модули: Core, GUI, QML, Quick, QuickWidgets, Widgets
- C++11 или выше
- GeoJSON файл с геометрией типа MultiPolygon

## Поддержка

При возникновении проблем проверьте:

1. Правильность путей в resources.qrc
2. Что `qmlRegisterType` вызван до создания QML
3. Что `setContextProperty` вызван до `setSource`
4. Консольный вывод для диагностики

## Лицензия

[Укажите вашу лицензию]
