# MapComponent - Интерактивный QML компонент карты

Компонент для отображения интерактивных географических карт на основе GeoJSON данных в Qt приложениях.

## Возможности

- Загрузка и отображение GeoJSON данных (MultiPolygon)
- Интерактивное взаимодействие: клики и наведение курсора
- Поддержка различных статусов регионов (default, warning, danger)
- Выделение выбранных регионов
- Автоматическое масштабирование и центрирование карты
- Адаптивный дизайн с изменением размера окна
- Оптимизированная производительность (throttle для hover, debounce для resize)
- Использование как в чистом QML, так и в QWidgets приложениях

## Структура проекта

```
qt-map-demo/
├── qml/
│   └── MapComponent.qml          # Основной QML компонент
├── data/
│   └── rus_simple_highcharts.geo.json  # GeoJSON данные карты
├── mapdata.h                     # C++ класс для работы с данными
├── mapdata.cpp
├── mainwindow.h                  # QWidgets интеграция
├── mainwindow.cpp
├── main.cpp                      # Точка входа
├── mainwindow.ui                 # UI форма
├── resources.qrc                 # Qt ресурсы
└── MapComponent.pro              # Проект файл
```

## Зависимости

- Qt 5.11+
- Qt модули: Core, GUI, QML, Quick, QuickWidgets, Widgets

## C++ API

### MapData класс

Основной класс для управления данными карты. Наследуется от `QObject` для поддержки системы сигналов-слотов Qt.

```cpp
class MapData : public QObject
{
    Q_OBJECT  // Макрос Qt для поддержки мета-объектной системы
    Q_PROPERTY(QVariantList regions READ regions NOTIFY regionsChanged)
    Q_PROPERTY(QString selectedRegion READ selectedRegion WRITE setSelectedRegion NOTIFY selectedRegionChanged)
    // ...
};
```

### Макросы Qt

#### Q_OBJECT

```cpp
Q_OBJECT
```

**Назначение:** Обязательный макрос для любого класса Qt, использующего систему сигналов и слотов, свойства или другие возможности мета-объектной системы Qt.

**Что делает:**
- Включает поддержку сигналов и слотов
- Позволяет использовать `Q_PROPERTY` для создания свойств QML
- Активирует механизм `QMetaObject` для рефлексии времени выполнения
- Позволяет использовать `qobject_cast<>` для безопасного приведения типов
- Необходим для работы с `QObject::connect()` и системой событий Qt

**Важно:** После добавления `Q_OBJECT` необходимо запустить `moc` (Meta-Object Compiler), который генерирует дополнительный код. qmake делает это автоматически.

#### Q_PROPERTY

```cpp
Q_PROPERTY(тип имя READ геттер [WRITE сеттер] [NOTIFY сигнал])
```

**Назначение:** Создает свойство, доступное из QML и через систему мета-объектов Qt.

**Пример 1 - Свойство только для чтения:**
```cpp
Q_PROPERTY(QVariantList regions READ regions NOTIFY regionsChanged)
```

- **QVariantList regions** - тип и имя свойства
- **READ regions** - геттер-метод `QVariantList regions() const`
- **NOTIFY regionsChanged** - сигнал, испускаемый при изменении значения

**Использование в QML:**
```qml
ListView {
    model: mapData.regions  // Автоматически вызывает regions()
    // При изменении испускается regionsChanged()
}
```

**Пример 2 - Свойство для чтения и записи:**
```cpp
Q_PROPERTY(QString selectedRegion READ selectedRegion WRITE setSelectedRegion NOTIFY selectedRegionChanged)
```

- **READ selectedRegion** - геттер `QString selectedRegion() const`
- **WRITE setSelectedRegion** - сеттер `void setSelectedRegion(const QString&)`
- **NOTIFY selectedRegionChanged** - сигнал изменения

**Использование в QML:**
```qml
// Чтение
Text { text: mapData.selectedRegion }

// Запись
Button {
    onClicked: mapData.selectedRegion = "10312"
}

// Привязка к сигналу
Connections {
    target: mapData
    function onSelectedRegionChanged(regionId) {
        console.log("Изменился:", regionId)
    }
}
```

#### Q_INVOKABLE

```cpp
Q_INVOKABLE void loadGeoJSON(const QString &filePath);
```

**Назначение:** Делает метод C++ доступным для вызова из QML без необходимости объявлять его как слот.

**Отличие от слота:**
- Слот (`public slots:`) может быть подключен к сигналу через `connect()`
- `Q_INVOKABLE` метод просто вызывается из QML, но не может быть подключен к сигналу
- `Q_INVOKABLE` чуть быстрее слота при вызове из QML

**Использование в QML:**
```qml
MapData {
    id: mapData
    Component.onCompleted: {
        // Прямой вызов метода C++
        loadGeoJSON("rus_simple_highcharts.geo.json")
    }
}
```

**Когда использовать:**
- Для методов, которые нужно вызывать из QML
- Когда не требуется подключение через `connect()`
- Для действий по требованию (загрузка данных, вычисления и т.д.)

### Свойства класса

#### regions (только чтение)

```cpp
Q_PROPERTY(QVariantList regions READ regions NOTIFY regionsChanged)

QVariantList regions() const { return m_regions; }
```

**Назначение:** Предоставляет список всех регионов карты для отображения в QML.

**Тип данных:** `QVariantList` - динамический список Qt, который может хранить любые типы данных. В нашем случае содержит `QVariantMap` объекты.

**Почему READ-only:** Изменение регионов должно происходить только через C++ методы (например, `loadGeoJSON()`), чтобы гарантировать правильность данных и согласованность геометрии.

**Сигнал regionsChanged():** Испускается после загрузки новых данных через `emit regionsChanged()`. QML автоматически обновляет все привязки к этому свойству.

#### selectedRegion (чтение и запись)

```cpp
Q_PROPERTY(QString selectedRegion READ selectedRegion WRITE setSelectedRegion NOTIFY selectedRegionChanged)

QString selectedRegion() const { return m_selectedRegion; }
void setSelectedRegion(const QString &regionId);
```

**Назначение:** Хранит ID текущего выбранного региона. Позволяет выделить регион на карте и синхронизировать выбор между QML и C++.

**Паттерн геттер/сеттер:**
```cpp
void MapData::setSelectedRegion(const QString &regionId)
{
    if (m_selectedRegion != regionId)  // Важная проверка!
    {
        m_selectedRegion = regionId;
        emit selectedRegionChanged(regionId);  // Оповещаем об изменении
        qDebug() << "Выбран регион:" << regionId;
    }
}
```

**Зачем проверка `if (m_selectedRegion != regionId)`:**
- Предотвращает бесконечные циклы при двусторонней привязке
- Избегает лишних перерисовок карты
- Не испускает сигнал, если значение не изменилось

### Методы класса

#### loadGeoJSON()

```cpp
Q_INVOKABLE void loadGeoJSON(const QString &filePath);
```

**Назначение:** Загружает и парсит GeoJSON файл, преобразуя географические координаты в SVG пути для отображения на Canvas.

**Параметры:**
- `filePath` - имя файла относительно `:/data/` (префикс ресурсов Qt)

**Процесс работы:**
1. Открывает файл из Qt ресурсов (`QFile(":/data/" + filePath)`)
2. Читает JSON данные (`QJsonDocument::fromJson()`)
3. Извлекает features из GeoJSON
4. Находит границы координат (min/max X/Y)
5. Нормализует координаты к размеру 1000x800 пикселей
6. Преобразует MultiPolygon в SVG path строки
7. Создает список регионов с метаданными
8. Испускает `regionsChanged()` для обновления QML

**Пример вызова:**
```cpp
// C++
mapData->loadGeoJSON("rus_simple_highcharts.geo.json");
```

```qml
// QML
Component.onCompleted: {
    mapData.loadGeoJSON("rus_simple_highcharts.geo.json")
}
```

### Сигналы класса

Сигналы в Qt - это механизм оповещения о событиях. Они автоматически создаются компилятором moc и могут быть подключены к слотам или обработаны в QML.

#### regionsChanged()

```cpp
void regionsChanged();
```

**Назначение:** Оповещает об изменении списка регионов (загрузка новых данных, изменение статусов).

**Когда испускается:** После успешной загрузки GeoJSON через `loadGeoJSON()`.

**Что происходит при испускании:**
1. Все QML привязки к `mapData.regions` обновляются автоматически
2. Canvas перерисовывается через `Connections { onRegionsChanged: mapCanvas.requestPaint() }`
3. ListView и другие модели данных обновляют отображение

**Использование в QML:**
```qml
Connections {
    target: mapData
    function onRegionsChanged() {
        console.log("Новые регионы загружены")
        mapCanvas.requestPaint()
    }
}
```

**Использование в C++:**
```cpp
// Подключение в конструкторе
connect(mapData, &MapData::regionsChanged, 
        this, &MainWindow::onRegionsUpdated);

// Слот в MainWindow
void MainWindow::onRegionsUpdated()
{
    qDebug() << "Регионы обновлены!";
    updateRegionsList();
}
```

#### selectedRegionChanged(const QString &regionId)

```cpp
void selectedRegionChanged(const QString &regionId);
```

**Назначение:** Оповещает об изменении выбранного региона. Передает ID региона как параметр.

**Когда испускается:** 
- При установке `selectedRegion` из QML: `mapData.selectedRegion = "10312"`
- При вызове `setSelectedRegion()` из C++
- При клике на регион в MapComponent

**Параметры:**
- `regionId` - ID выбранного региона (hc-key из GeoJSON)

**Использование в QML:**
```qml
Connections {
    target: mapData
    function onSelectedRegionChanged(regionId) {
        console.log("Выбран регион:", regionId)
        // Обновить информационную панель
        infoPanel.loadRegionDetails(regionId)
        // Подсветить в списке
        regionListView.currentIndex = findRegionIndex(regionId)
    }
}
```

**Использование в C++ (QWidgets):**
```cpp
// В конструкторе MainWindow
connect(mapData, &MapData::selectedRegionChanged,
        this, &MainWindow::onRegionSelected);

// Обработчик
void MainWindow::onRegionSelected(const QString &regionId)
{
    qDebug() << "Region selected:" << regionId;
    
    // Найти регион в списке
    QVariantList regions = mapData->regions();
    for (const QVariant &var : regions) {
        QVariantMap region = var.toMap();
        if (region["id"].toString() == regionId) {
            QString name = region["name"].toString();
            QString status = region["status"].toString();
            
            // Обновить UI
            ui->regionNameLabel->setText(name);
            ui->statusLabel->setText(status);
            
            // Выделить в таблице
            highlightRegionInTable(regionId);
            break;
        }
    }
}
```

#### regionStatusChanged(const QString &regionId, const QString &status)

```cpp
void regionStatusChanged(const QString &regionId, const QString &regionStatus);
```

**Назначение:** Оповещает об изменении статуса отдельного региона без перезагрузки всей карты.

**Когда испускается:** Когда нужно изменить статус региона (например, при обновлении данных с сервера или из БД).

**Параметры:**
- `regionId` - ID региона
- `regionStatus` - новый статус ("default", "warning", "danger")

**Использование (требуется реализация в MapData):**
```cpp
void MapData::updateRegionStatus(const QString &regionId, const QString &status)
{
    for (int i = 0; i < m_regions.size(); ++i) {
        QVariantMap region = m_regions[i].toMap();
        if (region["id"].toString() == regionId) {
            region["status"] = status;
            m_regions[i] = region;
            
            emit regionStatusChanged(regionId, status);
            emit regionsChanged();  // Для перерисовки
            break;
        }
    }
}
```

### Слоты в Qt

Хотя в текущей реализации MapData не использует явные слоты (`public slots:`), вот как они работают:

```cpp
class MapData : public QObject
{
    Q_OBJECT

public slots:
    void updateRegion(const QString &regionId, const QString &status);
    void clearSelection();
    
signals:
    void regionUpdated(const QString &regionId);
};
```

**Назначение слотов:**
- Методы, которые могут быть подключены к сигналам через `connect()`
- Могут вызываться из QML (как Q_INVOKABLE)
- Обычный C++ метод с дополнительной мета-информацией

**Подключение сигнал-слот:**
```cpp
// Новый синтаксис (рекомендуется, проверка типов на этапе компиляции)
connect(sender, &SenderClass::signalName,
        receiver, &ReceiverClass::slotName);

// Старый синтаксис (проверка только во время выполнения)
connect(sender, SIGNAL(signalName()),
        receiver, SLOT(slotName()));
```

**Пример использования:**
```cpp
// Автоматическое обновление при изменении
connect(mapData, &MapData::selectedRegionChanged,
        detailsPanel, &DetailsPanel::loadRegion);

// Связь между виджетами
connect(ui->regionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onRegionIndexChanged);
```

#### Структура региона

Каждый регион представлен `QVariantMap` со следующими полями:

```cpp
{
    "name": "Название региона",
    "id": "10312",              // hc-key из GeoJSON
    "postal-code": "123456",
    "paths": ["M 100 200 L 150 250 Z", ...],  // SVG пути
    "status": "default|warning|danger"
}
```

## QML API

### MapComponent

Основной QML компонент для отображения карты.

#### Свойства

##### Цвета фона и обводки

```qml
property color backgroundColor: "#f5f5f5"    // Фон компонента
property color borderColor: "#bdc3c7"        // Цвет рамки
property color strokeColor: "#ffffff"        // Цвет обводки регионов
property color activeStrokeColor: "#000000"  // Цвет обводки выбранного региона
property real strokeWidth: 1                 // Ширина обводки
property real activeStrokeWidth: 2           // Ширина обводки выбранного региона
```

##### Цвета регионов (неактивные)

```qml
property color defaultColor: "#5CA8FF"       // Обычный статус
property color warningColor: "#FFB84D"       // Предупреждение
property color dangerColor: "#FF5C5C"        // Опасность
```

##### Цвета регионов (активные/выбранные)

```qml
property color defaultActiveColor: "#1E7FFF"
property color warningActiveColor: "#FF9500"
property color dangerActiveColor: "#E53935"
```

##### Отступы и размеры

```qml
property int containerMargin: 10    // Отступ от краев контейнера
property int canvasPadding: 20      // Внутренний отступ для карты
```

#### Сигналы

Сигналы в QML работают аналогично сигналам Qt C++. Они оповещают другие компоненты о событиях.

##### regionClicked(string regionId, string regionName)

```qml
signal regionClicked(string regionId, string regionName)
```

**Назначение:** Испускается при клике пользователя на регион карты.

**Параметры:**
- `regionId` - уникальный идентификатор региона (hc-key)
- `regionName` - человекочитаемое название региона

**Когда испускается:**
- При левом клике мыши на область региона
- После определения региона через ray-casting алгоритм
- Только если клик попал в границы региона (не на пустую область)

**Внутренняя реализация:**
```qml
MouseArea {
    onClicked: function(mouse) {
        var clickedRegion = mapCanvas.getRegionAtPoint(mouse.x, mouse.y)
        if (clickedRegion) {
            mapData.selectedRegion = clickedRegion.id  // Выделяем регион
            mapComponent.regionClicked(clickedRegion.id, clickedRegion.name)
        }
    }
}
```

**Использование:**
```qml
MapComponent {
    onRegionClicked: function(regionId, regionName) {
        console.log("Клик по региону:", regionName, "ID:", regionId)
        
        // Отобразить детальную информацию
        detailsDialog.open()
        detailsDialog.loadRegion(regionId)
        
        // Обновить текстовое поле
        statusText.text = "Выбран: " + regionName
        
        // Загрузить данные с сервера
        networkManager.fetchRegionData(regionId)
    }
}
```

**Типичные сценарии:**
- Открытие панели с деталями региона
- Загрузка дополнительных данных
- Обновление статистики
- Навигация к другому экрану

##### regionHovered(string regionId, string regionName)

```qml
signal regionHovered(string regionId, string regionName)
```

**Назначение:** Испускается при наведении курсора мыши на регион.

**Параметры:**
- `regionId` - ID региона под курсором
- `regionName` - название региона

**Когда испускается:**
- При движении мыши над областью региона
- С throttle-оптимизацией: не чаще 16мс (60 FPS)
- Только при смене региона (не повторяется для одного региона)

**Оптимизация производительности:**
```qml
Timer {
    id: hoverThrottle
    interval: 16  // 60 FPS
    repeat: false
    
    onTriggered: {
        var hoveredRegion = mapCanvas.getRegionAtPoint(pendingX, pendingY)
        if (hoveredRegion && hoveredRegion.id !== currentHoveredRegion.id) {
            mapComponent.regionHovered(hoveredRegion.id, hoveredRegion.name)
        }
    }
}
```

**Использование:**
```qml
MapComponent {
    onRegionHovered: function(regionId, regionName) {
        // Показать всплывающую подсказку
        tooltip.text = regionName
        tooltip.visible = true
        
        // Обновить строку состояния
        statusBar.showMessage("Регион: " + regionName)
        
        // Загрузить превью данных
        regionPreview.loadQuickInfo(regionId)
    }
}

// Пример с ToolTip
ToolTip {
    id: tooltip
    visible: false
    delay: 500        // Задержка перед показом
    timeout: 3000     // Автоскрытие через 3 сек
}
```

**Типичные сценарии:**
- Отображение tooltip с названием
- Показ краткой информации без клика
- Подсветка соответствующего элемента в списке
- Загрузка preview данных

##### regionExited()

```qml
signal regionExited()
```

**Назначение:** Испускается когда курсор покидает область любого региона или выходит за пределы MapComponent.

**Параметры:** Нет

**Когда испускается:**
- Когда мышь перемещается с региона на пустую область карты
- Когда курсор покидает весь компонент MapComponent
- После проверки через ray-casting показывающей, что курсор не над регионом

**Внутренняя логика:**
```qml
MouseArea {
    onPositionChanged: function(mouse) {
        var hoveredRegion = mapCanvas.getRegionAtPoint(mouse.x, mouse.y)
        
        if (!hoveredRegion && currentHoveredRegion) {
            // Курсор покинул регион
            currentHoveredRegion = null
            mapComponent.regionExited()
        }
    }
    
    onExited: {
        // Курсор покинул весь MapComponent
        currentHoveredRegion = null
        mapComponent.regionExited()
    }
}
```

**Использование:**
```qml
MapComponent {
    onRegionExited: {
        // Скрыть tooltip
        tooltip.visible = false
        
        // Очистить строку состояния
        statusBar.clearMessage()
        
        // Вернуть обычный курсор
        cursorShape = Qt.ArrowCursor
        
        // Очистить preview панель
        regionPreview.clear()
        
        // Восстановить текст по умолчанию
        if (mapData.selectedRegion === "") {
            statusText.text = "Наведите на регион"
        }
    }
}
```

**Типичные сценарии:**
- Скрытие временных UI элементов
- Очистка preview панелей
- Восстановление курсора по умолчанию
- Сброс hover-эффектов

#### Координация между сигналами

**Последовательность событий при взаимодействии:**

1. **Наведение на регион:**
   ```
   regionHovered("10312", "Красноярский край")
   → Tooltip появляется
   → Курсор меняется на PointingHandCursor
   ```

2. **Клик по региону:**
   ```
   regionClicked("10312", "Красноярский край")
   → mapData.selectedRegion = "10312"
   → selectedRegionChanged("10312") испускается из C++
   → Canvas перерисовывается с выделением
   → Детальная панель обновляется
   ```

3. **Уход с региона:**
   ```
   regionExited()
   → Tooltip скрывается
   → Курсор возвращается в ArrowCursor
   → Но выделение остается (т.к. selectedRegion не изменился)
   ```

**Пример комплексной обработки:**
```qml
Item {
    MapComponent {
        id: mapComponent
        
        onRegionHovered: function(regionId, regionName) {
            // Быстрая обратная связь
            tooltip.text = regionName
            tooltip.visible = true
            hoverStatus.text = regionName
        }
        
        onRegionClicked: function(regionId, regionName) {
            // Полное взаимодействие
            console.log("Клик:", regionName)
            
            // Обновить выбор (автоматически вызовет перерисовку)
            mapData.selectedRegion = regionId
            
            // Открыть детали
            detailsPanel.visible = true
            detailsPanel.regionId = regionId
            
            // Обновить историю
            navigationHistory.push(regionId)
        }
        
        onRegionExited: {
            // Очистить hover-эффекты, но сохранить выделение
            tooltip.visible = false
            if (mapData.selectedRegion === "") {
                hoverStatus.text = "Наведите курсор на регион"
            }
        }
    }
    
    // Tooltip
    ToolTip {
        id: tooltip
        visible: false
    }
    
    // Строка состояния
    Text {
        id: hoverStatus
        anchors.bottom: parent.bottom
        text: "Наведите курсор на регион"
    }
}

#### Пример использования в чистом QML

```qml
import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import MapData 1.0

Window {
    visible: true
    width: 1024
    height: 768
    title: "Карта России"

    MapData {
        id: mapData
        Component.onCompleted: {
            loadGeoJSON("rus_simple_highcharts.geo.json")
        }
    }

    Column {
        anchors.fill: parent
        spacing: 10
        padding: 10

        // Информационная панель
        Rectangle {
            width: parent.width - 20
            height: 50
            color: "#ecf0f1"
            radius: 5

            Text {
                id: infoText
                anchors.centerIn: parent
                text: "Наведите на регион или кликните"
                font.pixelSize: 16
            }
        }

        // Компонент карты
        Rectangle {
            width: parent.width - 20
            height: parent.height - 80
            border.color: "#bdc3c7"
            border.width: 1

            MapComponent {
                id: mapComponent
                anchors.fill: parent

                // Настройка цветов
                backgroundColor: "#ffffff"
                defaultColor: "#5CA8FF"
                warningColor: "#FFB84D"
                dangerColor: "#FF5C5C"

                // Обработка событий
                onRegionClicked: function(regionId, regionName) {
                    console.log("Клик:", regionName, regionId)
                    infoText.text = "Выбран: " + regionName + " (ID: " + regionId + ")"
                }

                onRegionHovered: function(regionId, regionName) {
                    infoText.text = "Наведение: " + regionName
                }

                onRegionExited: {
                    if (mapData.selectedRegion === "") {
                        infoText.text = "Наведите на регион или кликните"
                    }
                }
            }
        }
    }
}
```

#### Пример главного файла main.cpp для чистого QML

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "mapdata.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Регистрируем C++ тип в QML
    qmlRegisterType<MapData>("MapData", 1, 0, "MapData");

    QQmlApplicationEngine engine;

    // Или можно создать экземпляр и передать через контекст
    // MapData *mapData = new MapData(&app);
    // mapData->loadGeoJSON("rus_simple_highcharts.geo.json");
    // engine.rootContext()->setContextProperty("mapData", mapData);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
```

## Использование в QWidgets приложении

### Механизм интеграции QML в QWidgets

Qt предоставляет `QQuickWidget` - виджет для встраивания QML контента в традиционное QWidgets приложение.

```cpp
#include <QQuickWidget>  // Виджет-контейнер для QML

QQuickWidget *quickWidget = new QQuickWidget(parent);
quickWidget->setSource(QUrl("qrc:/qml/MapComponent.qml"));
quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
```

**Преимущества QQuickWidget:**
- Бесшовная интеграция QML в QWidgets UI
- Возможность использовать Qt Designer
- Доступ к QML контексту из C++
- Поддержка layouts и всех виджетных механизмов

### 1. Добавьте QQuickWidget в форму

**Способ 1: Через Qt Designer (рекомендуется)**

1. Откройте `mainwindow.ui` в Qt Designer
2. Перетащите `QWidget` из палитры виджетов на форму
3. Правой кнопкой мыши → "Преобразовать в..." → "QQuickWidget"
4. Установите objectName (например, `quickWidget`)
5. Настройте geometry и layout

**Способ 2: Программно в конструкторе**

```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // Создаем QQuickWidget программно
    QQuickWidget *quickWidget = new QQuickWidget(this);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    
    // Добавляем в layout
    ui->centralLayout->addWidget(quickWidget);
    
    // Сохраняем указатель для дальнейшего использования
    m_quickWidget = quickWidget;
}
```

**setResizeMode() - важная настройка:**

```cpp
enum ResizeMode {
    SizeViewToRootObject,  // Размер виджета подгоняется под QML
    SizeRootObjectToView   // QML подгоняется под размер виджета (обычно нужен этот)
};
```

### 2. Регистрация C++ типов в QML

Qt предоставляет несколько способов сделать C++ классы доступными в QML:

#### Способ 1: qmlRegisterType (регистрация типа)

```cpp
#include <QtQml>  // Для qmlRegisterType

// В main.cpp или MainWindow конструкторе
qmlRegisterType<MapData>("MapData", 1, 0, "MapData");
//              ^^^^^^^^  ^^^^^^^^  ^^^^  ^^^^^^^^
//              C++ класс  Модуль   Версия  QML имя
```

**Что это делает:**
- Регистрирует C++ класс как QML тип
- Позволяет создавать экземпляры из QML: `MapData { id: mapData }`
- Первые два параметра - идентификатор модуля для import
- Версия (major, minor) для управления совместимостью
- Последний параметр - имя типа в QML

**Использование в QML:**
```qml
import MapData 1.0  // Импорт модуля

Window {
    MapData {
        id: mapData  // Создание экземпляра
        Component.onCompleted: {
            loadGeoJSON("data.json")
        }
    }
}
```

**Когда использовать:**
- Для чистых QML приложений
- Когда нужно создавать множество экземпляров
- Для многоразовых компонентов

#### Способ 2: setContextProperty (контекстное свойство)

```cpp
#include <QQmlContext>

// Создаем экземпляр в C++
MapData *mapData = new MapData(this);
mapData->loadGeoJSON("rus_simple_highcharts.geo.json");

// Передаем в QML через контекст
ui->quickWidget->rootContext()->setContextProperty("mapData", mapData);
//                ^^^^^^^^^^^                      ^^^^^^^^   ^^^^^^^
//                QML контекст                     Имя в QML  C++ объект
```

**Что это делает:**
- Делает существующий C++ объект доступным в QML как глобальная переменная
- Экземпляр создается и управляется из C++
- Один объект доступен во всем QML коде

**Использование в QML:**
```qml
// НЕ нужен import, mapData уже доступен глобально
Window {
    Text {
        text: mapData.selectedRegion  // Прямое использование
    }
    
    Component.onCompleted: {
        console.log(mapData.regions.length)
    }
}
```

**Когда использовать:**
- Для QWidgets + QML интеграции (наш случай)
- Когда нужен один глобальный экземпляр
- Когда C++ код управляет жизненным циклом объекта
- Для передачи данных из C++ в QML

**Важный порядок вызовов:**
```cpp
// 1. Сначала регистрируем тип (если нужно)
qmlRegisterType<MapData>("MapData", 1, 0, "MapData");

// 2. Создаем экземпляр
MapData *mapData = new MapData(this);

// 3. Настраиваем объект
mapData->loadGeoJSON("data.json");

// 4. Передаем в контекст ДО загрузки QML!
ui->quickWidget->rootContext()->setContextProperty("mapData", mapData);

// 5. Загружаем QML (последним!)
ui->quickWidget->setSource(QUrl("qrc:/qml/MapComponent.qml"));
```

**Почему порядок важен:**
- QML код компилируется при `setSource()`
- Если `mapData` не в контексте, получим ошибку "mapData is not defined"
- Контекст должен быть готов до парсинга QML

### 3. Полный пример MainWindow

**mainwindow.h:**

```cpp
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mapdata.h"  // Наш C++ класс

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT  // Макрос для сигналов/слотов

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слоты для обработки сигналов от MapData
    void onRegionSelected(const QString &regionId);
    void onRegionsLoaded();

private:
    Ui::MainWindow *ui;      // UI форма из Qt Designer
    MapData *m_mapData;       // Наш объект данных
    
    // Вспомогательные методы
    void setupMapComponent();
    void connectSignals();
};

#endif // MAINWINDOW_H
```

**mainwindow.cpp:**

```cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QQmlContext>
#include <QtQml>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_mapData(nullptr)
{
    ui->setupUi(this);
    
    // Настраиваем компонент карты
    setupMapComponent();
    
    // Подключаем сигналы
    connectSignals();
}

MainWindow::~MainWindow()
{
    delete ui;
    // m_mapData удалится автоматически (parent = this)
}

void MainWindow::setupMapComponent()
{
    // 1. Регистрируем C++ тип для использования в QML
    //    Это нужно сделать ОДИН РАЗ в приложении
    qmlRegisterType<MapData>("MapData", 1, 0, "MapData");
    
    // 2. Создаем экземпляр MapData
    //    parent = this обеспечивает автоматическое удаление
    m_mapData = new MapData(this);
    
    // 3. Загружаем данные карты из ресурсов
    //    Путь относительно qrc:/data/
    m_mapData->loadGeoJSON("rus_simple_highcharts.geo.json");
    
    // 4. ВАЖНО: Передаем объект в QML контекст ДО загрузки QML
    //    rootContext() - корневой контекст для всего QML кода
    ui->quickWidget->rootContext()->setContextProperty("mapData", m_mapData);
    
    // 5. Настраиваем режим изменения размера
    //    SizeRootObjectToView - QML компонент растягивается под виджет
    ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    
    // 6. Загружаем QML файл из ресурсов
    //    ПОСЛЕ setContextProperty!
    ui->quickWidget->setSource(QUrl("qrc:/qml/MapComponent.qml"));
    
    // 7. Проверка на ошибки загрузки
    if (ui->quickWidget->status() == QQuickWidget::Error) {
        qCritical() << "Ошибка загрузки QML:";
        for (const QQmlError &error : ui->quickWidget->errors()) {
            qCritical() << error.toString();
        }
    }
}

void MainWindow::connectSignals()
{
    // Подключаем сигналы MapData к слотам MainWindow
    
    // Новый синтаксис connect (рекомендуется)
    // Преимущества: проверка типов на этапе компиляции, поддержка лямбд
    connect(m_mapData, &MapData::selectedRegionChanged,
            this, &MainWindow::onRegionSelected);
    
    connect(m_mapData, &MapData::regionsChanged,
            this, &MainWindow::onRegionsLoaded);
    
    // Пример с лямбда-функцией
    connect(m_mapData, &MapData::regionsChanged, this, [this]() {
        qDebug() << "Загружено регионов:" << m_mapData->regions().size();
        ui->statusBar->showMessage(
            QString("Загружено %1 регионов").arg(m_mapData->regions().size()),
            3000  // Показать на 3 секунды
        );
    });
}

void MainWindow::onRegionSelected(const QString &regionId)
{
    qDebug() << "Region selected in C++:" << regionId;
    
    // Найти полную информацию о регионе
    QVariantList regions = m_mapData->regions();
    for (const QVariant &var : regions) {
        QVariantMap region = var.toMap();
        
        if (region["id"].toString() == regionId) {
            QString name = region["name"].toString();
            QString status = region["status"].toString();
            QString postalCode = region["postal-code"].toString();
            
            // Обновить UI виджеты
            ui->regionNameLabel->setText(name);
            ui->regionIdLabel->setText(regionId);
            ui->statusLabel->setText(status);
            
            // Изменить цвет статуса
            if (status == "danger") {
                ui->statusLabel->setStyleSheet("color: #E53935; font-weight: bold;");
            } else if (status == "warning") {
                ui->statusLabel->setStyleSheet("color: #FF9500; font-weight: bold;");
            } else {
                ui->statusLabel->setStyleSheet("color: #1E7FFF;");
            }
            
            // Обновить статус-бар
            ui->statusBar->showMessage(QString("Выбран: %1").arg(name));
            
            // Можно вызвать другие методы
            loadRegionStatistics(regionId);
            updateRegionChart(regionId);
            
            break;
        }
    }
}

void MainWindow::onRegionsLoaded()
{
    qDebug() << "Regions loaded callback";
    
    // Обновить список регионов в QListWidget или QTableWidget
    ui->regionsListWidget->clear();
    
    QVariantList regions = m_mapData->regions();
    for (const QVariant &var : regions) {
        QVariantMap region = var.toMap();
        QString name = region["name"].toString();
        QString id = region["id"].toString();
        
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, id);  // Сохраняем ID в item
        ui->regionsListWidget->addItem(item);
    }
}

// Пример взаимодействия из QWidgets в QML
void MainWindow::on_selectRegionButton_clicked()
{
    // Программно выбрать регион в карте
    QString regionId = ui->regionIdLineEdit->text();
    m_mapData->setSelectedRegion(regionId);
    // Карта автоматически обновится через сигнал selectedRegionChanged
}
```

### 4. Двусторонняя коммуникация

**C++ → QML (через setContextProperty):**
```cpp
// Изменение в C++ автоматически отражается в QML
m_mapData->setSelectedRegion("10312");
// QML получает сигнал и обновляет отображение
```

**QML → C++ (через сигналы):**

**Способ 1: Подключение к сигналам MapData**
```cpp
connect(m_mapData, &MapData::selectedRegionChanged, 
        this, &MainWindow::onRegionSelected);
```

**Способ 2: Доступ к корневому QML объекту**
```cpp
// Получить корневой объект QML
QObject *rootObject = ui->quickWidget->rootObject();

// Найти конкретный компонент по objectName
QObject *mapComponent = rootObject->findChild<QObject*>("mapComponent");

if (mapComponent) {
    // Подключиться к QML сигналу
    connect(mapComponent, SIGNAL(regionClicked(QString, QString)),
            this, SLOT(onQmlRegionClicked(QString, QString)));
}

// Слот в MainWindow
void MainWindow::onQmlRegionClicked(const QString &regionId, 
                                     const QString &regionName)
{
    qDebug() << "QML signal received:" << regionName;
}
```

**QML компонент с objectName:**
```qml
MapComponent {
    objectName: "mapComponent"  // Для поиска из C++
    
    onRegionClicked: function(regionId, regionName) {
        // Этот сигнал можно поймать в C++
    }
}
```

### 5. Обработка ошибок

```cpp
void MainWindow::setupMapComponent()
{
    // ... настройка ...
    
    ui->quickWidget->setSource(QUrl("qrc:/qml/MapComponent.qml"));
    
    // Проверка статуса загрузки
    QQuickWidget::Status status = ui->quickWidget->status();
    
    switch (status) {
        case QQuickWidget::Null:
            qWarning() << "QML не загружен";
            break;
            
        case QQuickWidget::Ready:
            qDebug() << "QML успешно загружен";
            break;
            
        case QQuickWidget::Loading:
            qDebug() << "QML загружается...";
            break;
            
        case QQuickWidget::Error:
            qCritical() << "Ошибка загрузки QML:";
            // Вывести все ошибки
            for (const QQmlError &error : ui->quickWidget->errors()) {
                qCritical() << "  Строка:" << error.line()
                           << "Файл:" << error.url()
                           << "Описание:" << error.description();
            }
            
            // Показать пользователю
            QMessageBox::critical(this, "Ошибка", 
                "Не удалось загрузить компонент карты");
            break;
    }
}
```

### 3. Настройка .pro файла

**MapComponent.pro:**

```qmake
# Подключение необходимых модулей Qt
QT += core gui qml quick quickwidgets widgets
#     ^^^^ ^^^ ^^^ ^^^^^ ^^^^^^^^^^^ ^^^^^^^
#     Core GUI QML Quick QuickWidgets Widgets

# core       - базовые классы Qt (QObject, QString, QVariant и т.д.)
# gui        - базовый GUI функционал (QWindow, события мыши и т.д.)
# qml        - QML engine (QQmlEngine, QQmlContext)
# quick      - Qt Quick компоненты (Canvas, Rectangle, MouseArea)
# quickwidgets - интеграция QML в QWidgets (QQuickWidget)
# widgets    - классические виджеты (QMainWindow, QPushButton и т.д.)

# Проверка версии Qt (для условной компиляции)
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Имя исполняемого файла
TARGET = MapComponent

# Тип проекта (app = приложение, lib = библиотека)
TEMPLATE = app

# Флаги компилятора
DEFINES += QT_DEPRECATED_WARNINGS
# Предупреждает об использовании устаревших Qt API

# Опционально: ошибка компиляции при использовании deprecated API
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
# 0x060000 = Qt 6.0.0

# Стандарт C++
CONFIG += c++11
# Включает C++11 стандарт (auto, lambda, nullptr и т.д.)

# Исходные файлы C++
SOURCES += \
    main.cpp \         # Точка входа приложения
    mainwindow.cpp \   # Главное окно
    mapdata.cpp        # Класс работы с данными карты

# Заголовочные файлы
HEADERS += \
    mainwindow.h \     # Q_OBJECT классы требуют moc обработки
    mapdata.h

# UI формы Qt Designer
FORMS += \
    mainwindow.ui      # Компилируется в ui_mainwindow.h

# Qt ресурсы (QRC)
RESOURCES += \
    resources.qrc      # QML файлы, изображения, данные

# Путь для поиска QML модулей (не обязательно в нашем случае)
QML_IMPORT_PATH = .

# Дополнительные пути для qmldir файлов (для продвинутого использования)
# QML2_IMPORT_PATH =

# Конфигурация папок для артефактов сборки
# Это держит проект в чистоте, разделяя исходники и результаты компиляции

DESTDIR = bin
# Папка для исполняемого файла
# Результат: bin/MapComponent или bin/MapComponent.exe

OBJECTS_DIR = build/obj
# Папка для объектных файлов (.o или .obj)
# main.o, mapdata.o и т.д.

MOC_DIR = build/moc
# Папка для файлов Meta-Object Compiler
# moc_mainwindow.cpp, moc_mapdata.cpp
# moc обрабатывает Q_OBJECT макрос

RCC_DIR = build/rcc
# Папка для скомпилированных ресурсов
# qrc_resources.cpp

UI_DIR = build/ui
# Папка для скомпилированных UI форм
# ui_mainwindow.h

# Правила установки (deployment)
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# target.path - куда установить при 'make install'
# INSTALLS - список того, что нужно установить
```

**Зачем нужны разные папки сборки:**

1. **DESTDIR** - исполняемый файл легко найти
2. **OBJECTS_DIR** - объектные файлы не засоряют корень проекта
3. **MOC_DIR** - автогенерированный код отделен от исходного
4. **RCC_DIR** - скомпилированные ресурсы изолированы
5. **UI_DIR** - UI формы не смешиваются с заголовками

**Процесс сборки Qt проекта:**

```
1. qmake MapComponent.pro
   ├─> Генерирует Makefile из .pro файла
   └─> Анализирует зависимости

2. make (или nmake на Windows)
   ├─> uic mainwindow.ui -> build/ui/ui_mainwindow.h
   │   (User Interface Compiler)
   │
   ├─> moc mainwindow.h -> build/moc/moc_mainwindow.cpp
   │   (Meta-Object Compiler для Q_OBJECT)
   │
   ├─> rcc resources.qrc -> build/rcc/qrc_resources.cpp
   │   (Resource Compiler)
   │
   ├─> g++ main.cpp -> build/obj/main.o
   ├─> g++ mainwindow.cpp -> build/obj/mainwindow.o
   ├─> g++ mapdata.cpp -> build/obj/mapdata.o
   ├─> g++ moc_mainwindow.cpp -> build/obj/moc_mainwindow.o
   ├─> g++ moc_mapdata.cpp -> build/obj/moc_mapdata.o
   └─> g++ qrc_resources.cpp -> build/obj/qrc_resources.o
   
   └─> Линковка всех .o файлов -> bin/MapComponent
```

**Условная компиляция:**

```qmake
# Для разных платформ
win32 {
    # Только для Windows
    LIBS += -luser32
}

unix:!macx {
    # Только для Linux
    QMAKE_CXXFLAGS += -std=c++17
}

macx {
    # Только для macOS
    ICON = app.icns
}

# По типу сборки
CONFIG(debug, debug|release) {
    # Debug сборка
    DEFINES += DEBUG_MODE
    TARGET = MapComponent_debug
}

CONFIG(release, debug|release) {
    # Release сборка
    DEFINES += QT_NO_DEBUG_OUTPUT
}
```

**Добавление внешних библиотек:**

```qmake
# Подключение Qt SQL для работы с БД
QT += sql

# Системные библиотеки
LIBS += -lz  # zlib

# Путь к библиотеке
LIBS += -L/usr/local/lib -lmylibrary

# Путь к заголовочным файлам
INCLUDEPATH += /usr/local/include
```

### 4. Настройка ресурсов

**resources.qrc:**

Qt Resource System (QRC) позволяет встроить файлы в исполняемый файл приложения.

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <!-- Первая группа ресурсов: QML файлы -->
    <qresource>
        <file>qml/MapComponent.qml</file>
        <!-- file - относительный путь от .qrc файла -->
        <!-- Доступ: qrc:/qml/MapComponent.qml или :/qml/MapComponent.qml -->
    </qresource>
    
    <!-- Вторая группа: данные -->
    <qresource>
        <file>data/rus_simple_highcharts.geo.json</file>
        <!-- Доступ: :/data/rus_simple_highcharts.geo.json -->
    </qresource>
</RCC>
```

**Зачем нужна система ресурсов:**

1. **Единый исполняемый файл** - все ресурсы встроены, не нужно развертывать отдельные файлы
2. **Защита данных** - файлы компилируются в бинарный формат
3. **Кросс-платформенность** - одинаковые пути на всех ОС
4. **Быстрый доступ** - файлы уже в памяти, не требуется файловая система

**Префиксы ресурсов:**

```xml
<RCC version="1.0">
    <!-- С префиксом для организации -->
    <qresource prefix="/ui">
        <file>qml/MapComponent.qml</file>
        <!-- Доступ: qrc:/ui/qml/MapComponent.qml -->
    </qresource>
    
    <qresource prefix="/data">
        <file>data/map.json</file>
        <!-- Доступ: qrc:/data/data/map.json -->
    </qresource>
    
    <!-- Локализация для разных языков -->
    <qresource prefix="/translations">
        <file lang="ru">translations/app_ru.qm</file>
        <file lang="en">translations/app_en.qm</file>
    </qresource>
</RCC>
```

**Использование ресурсов в C++:**

```cpp
// Чтение файла из ресурсов
QFile file(":/data/rus_simple_highcharts.geo.json");
if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    file.close();
}

// Загрузка изображения
QPixmap pixmap(":/images/logo.png");

// Применение стилей CSS
QFile styleFile(":/styles/dark_theme.qss");
styleFile.open(QIODevice::ReadOnly);
QString style = QString::fromUtf8(styleFile.readAll());
qApp->setStyleSheet(style);
```

**Использование в QML:**

```qml
// Загрузка изображения
Image {
    source: "qrc:/images/icon.png"
}

// Загрузка компонента
Loader {
    source: "qrc:/qml/CustomButton.qml"
}

// Загрузка данных
Component.onCompleted: {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "qrc:/data/config.json");
    xhr.onreadystatechange = function() {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            var data = JSON.parse(xhr.responseText);
        }
    }
    xhr.send();
}
```

**Алиасы для упрощения путей:**

```xml
<RCC version="1.0">
    <qresource>
        <!-- Оригинальный файл -->
        <file>qml/MapComponent.qml</file>
        
        <!-- Алиас для короткого пути -->
        <file alias="MapComponent.qml">qml/MapComponent.qml</file>
        <!-- Доступ: :/MapComponent.qml -->
    </qresource>
</RCC>
```

**Компиляция ресурсов:**

```bash
# Автоматически через qmake/make
rcc resources.qrc -o qrc_resources.cpp
# Генерирует C++ код с данными в виде массива байт

# Бинарный ресурс (для динамической загрузки)
rcc resources.qrc -binary -o resources.rcc
```

```cpp
// Регистрация бинарного ресурса во время выполнения
QResource::registerResource("resources.rcc");
```

**Структура qrc_resources.cpp (автогенерируется):**

```cpp
static const unsigned char qt_resource_data[] = {
    // Сжатые данные файлов в виде байт
    0x51,0x4d,0x4c,0x20,0x66,0x69,0x6c,0x65, ...
};

static const unsigned char qt_resource_name[] = {
    // Имена файлов
    0x00,0x03, // Длина
    0x00,0x00,0x00,0x71, // Хеш
    'q','m','l', ...
};

static const unsigned char qt_resource_struct[] = {
    // Структура дерева ресурсов
    0x00,0x00,0x00,0x00, ...
};

// Регистрация ресурсов при старте программы
static void qRegisterResourceData(...) { ... }
static void qUnregisterResourceData(...) { ... }
```

**Преимущества и недостатки:**

**Плюсы:**
- Простота развертывания (один exe файл)
- Не нужны инсталляторы для мелких приложений
- Защита от случайного удаления файлов
- Кросс-платформенные пути

**Минусы:**
- Увеличивает размер исполняемого файла
- Нельзя изменить ресурсы без пересборки
- Медленнее для очень больших файлов (например, видео)

**Когда использовать файловую систему вместо QRC:**
- Пользовательские настройки (должны сохраняться)
- Большие медиа файлы
- Часто изменяемые данные
- Файлы, которые пользователь может захотеть изменить

**Гибридный подход:**

```cpp
// Попытка загрузить из файловой системы
QString path = "data/map.json";
QFile file(path);

// Если не найден, использовать встроенный ресурс
if (!file.exists()) {
    path = ":/data/map.json";
    file.setFileName(path);
}

if (file.open(QIODevice::ReadOnly)) {
    // Обработка данных
}
```

## Программное управление

### Изменение статуса регионов

```cpp
// В MapData::parseGeoJSON() или в отдельном методе
QString status = "default";

if (hcKey == "10312" || hcKey == "10401") {
    status = "warning";
} else if (hcKey == "10202" || hcKey == "10505") {
    status = "danger";
}

region["status"] = status;
```

### Выбор региона программно

```cpp
// Из C++
mapData->setSelectedRegion("10312");
```

```qml
// Из QML
mapData.selectedRegion = "10312"
```

### Получение информации о регионах

```qml
// В QML
ListView {
    model: mapData.regions
    delegate: Text {
        text: modelData.name + " - " + modelData.status
    }
}
```

```cpp
// В C++
QVariantList regions = mapData->regions();
for (const QVariant &var : regions) {
    QVariantMap region = var.toMap();
    qDebug() << region["name"].toString() 
             << region["status"].toString();
}
```

## Форматы данных

### GeoJSON

Компонент поддерживает GeoJSON с типом геометрии `MultiPolygon`:

```json
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {
        "hc-key": "10312",
        "name": "Красноярский край",
        "postal-code": "123456"
      },
      "geometry": {
        "type": "MultiPolygon",
        "coordinates": [
          [
            [
              [100.123, 55.456],
              [101.234, 56.567],
              [100.123, 55.456]
            ]
          ]
        ]
      }
    }
  ]
}
```

### Преобразование координат

Координаты автоматически преобразуются из GeoJSON в SVG пути:
- Нормализация к диапазону 1000x800 пикселей
- Инверсия оси Y (SVG координаты идут сверху вниз)
- Добавление padding для отступов

## Производительность

### Оптимизации

1. **Threaded Canvas Rendering** - рендеринг выполняется в отдельном потоке
2. **Hover Throttle** - проверка региона не чаще 60 FPS (16мс)
3. **Resize Debounce** - перерисовка с задержкой 150мс при изменении размера
4. **Bounding Box Test** - быстрая предварительная проверка перед ray casting
5. **Флаги isPainting** - предотвращение множественных перерисовок

### Рекомендации

- Для карт с большим количеством регионов (>100) рассмотрите упрощение геометрии
- Используйте `debugInfo.visible = true` для мониторинга производительности
- При необходимости увеличьте интервал `hoverThrottle.interval`

## Отладка

### Включение отладочной информации

В `MapComponent.qml` установите:

```qml
Rectangle {
    id: debugInfo
    // ...
    visible: true  // Изменить на true
}
```

Отображает:
- Количество регионов
- Количество отрисованных путей
- Текущий масштаб
- Смещение карты

### Консольные логи

Компонент выводит подробную информацию в консоль:

```
=== MapComponent инициализирован ===
Размеры компонента: 1000 x 800
MapData доступен, регионов: 85
=== Canvas onPaint вызван ===
Регионов для отрисовки: 85
Размер canvas: 980 x 780
Нарисовано путей: 95 из 95
```

## Кастомизация

### Изменение цветовой схемы

```qml
MapComponent {
    // Темная тема
    backgroundColor: "#2c3e50"
    strokeColor: "#34495e"
    activeStrokeColor: "#ecf0f1"
    
    defaultColor: "#3498db"
    warningColor: "#f39c12"
    dangerColor: "#e74c3c"
    
    defaultActiveColor: "#2980b9"
    warningActiveColor: "#d68910"
    dangerActiveColor: "#c0392b"
}
```

### Изменение размеров и отступов

```qml
MapComponent {
    containerMargin: 20      // Больше отступ от краев
    canvasPadding: 30        // Больше внутренний отступ
    strokeWidth: 2           // Толще обводка
    activeStrokeWidth: 3     // Толще обводка выбранного
}
```

### Добавление тултипов

```qml
MapComponent {
    id: mapComponent
    
    onRegionHovered: function(regionId, regionName) {
        tooltip.text = regionName
        tooltip.visible = true
    }
    
    onRegionExited: {
        tooltip.visible = false
    }
}

ToolTip {
    id: tooltip
    visible: false
    delay: 0
    timeout: 5000
}
```

## Примеры использования

### Интеграция с базой данных

```cpp
void MainWindow::loadRegionDataFromDB()
{
    // Загрузить статусы из БД
    QSqlQuery query("SELECT region_id, status FROM regions");
    
    while (query.next()) {
        QString regionId = query.value(0).toString();
        QString status = query.value(1).toString();
        
        // Обновить статус региона
        updateRegionStatus(regionId, status);
    }
}

void MainWindow::updateRegionStatus(const QString &regionId, 
                                   const QString &status)
{
    QVariantList regions = mapData->regions();
    
    for (int i = 0; i < regions.size(); ++i) {
        QVariantMap region = regions[i].toMap();
        if (region["id"].toString() == regionId) {
            region["status"] = status;
            regions[i] = region;
            break;
        }
    }
    
    // Обновить данные (потребуется добавить метод в MapData)
    // mapData->updateRegions(regions);
}
```

### Анимация изменений

```qml
MapComponent {
    id: mapComponent
    
    // Анимация цветов при выборе
    Behavior on defaultActiveColor {
        ColorAnimation { duration: 200 }
    }
    Behavior on warningActiveColor {
        ColorAnimation { duration: 200 }
    }
    Behavior on dangerActiveColor {
        ColorAnimation { duration: 200 }
    }
}
```

### Фильтрация регионов

```qml
ComboBox {
    model: ["Все", "Обычные", "Предупреждение", "Опасность"]
    
    onCurrentTextChanged: {
        var filter = currentText
        // Реализовать фильтрацию через MapData
        if (filter === "Предупреждение") {
            mapData.filterByStatus("warning")
        } else if (filter === "Опасность") {
            mapData.filterByStatus("danger")
        } else {
            mapData.clearFilter()
        }
    }
}
```

## Известные ограничения

1. Поддерживается только тип геометрии `MultiPolygon` из GeoJSON
2. Не поддерживаются кривые Безье в путях (только M, L, Z команды)
3. Масштабирование и панорамирование карты не реализованы (только отображение)
4. Один выбранный регион за раз

## Лицензия

Укажите вашу лицензию здесь.

## Контакты

Укажите контактную информацию или ссылки на репозиторий.
