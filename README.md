# MapComponent - Переиспользуемый компонент карты России

## Описание

`MapComponent.qml` - это отдельный QML компонент для отображения интерактивной карты России, который можно использовать как в чистых QML приложениях, так и в Qt Widgets через `QQuickWidget`.

## Возможности компонента

- ✅ Отображение регионов России из GeoJSON
- ✅ Интерактивные клики по регионам
- ✅ Отслеживание наведения мыши
- ✅ Изменение цветов регионов
- ✅ Выделение выбранного региона
- ✅ Настраиваемые цвета и размеры
- ✅ Сигналы для обработки событий

## Структура файлов

```
qt-map-demo/
├── MapComponent.qml           # Основной компонент карты (переиспользуемый)
├── map.qml                    # Пример использования в QML приложении
├── widget_map_example.h       # Пример использования в Qt Widgets
├── mapdata.h/cpp              # C++ backend для работы с данными
├── main.cpp                   # Точка входа приложения
└── resources.qrc              # Ресурсы Qt
```

## Использование в QML приложении

### Базовое использование

```qml
import QtQuick 2.12

Item {
    MapComponent {
        id: myMap
        anchors.fill: parent
        
        mapData: myMapDataObject  // Объект MapData из C++
        
        onRegionClicked: {
            console.log("Клик:", regionId, regionName)
        }
    }
}
```

### Полный пример с обработкой событий

```qml
MapComponent {
    id: mapComponent
    width: 1000
    height: 800
    
    // Свойства
    mapData: mapDataBackend
    selectedRegion: "moscow"
    canvasWidth: 1200
    canvasHeight: 1000
    backgroundColor: "#ffffff"
    selectedRegionColor: "#e74c3c"
    
    // События
    onRegionClicked: {
        console.log("Выбран регион:", regionName, "(ID:", regionId, ")")
        // Ваша логика обработки
    }
    
    onRegionHovered: {
        tooltipText.text = regionName
    }
    
    onRegionExited: {
        tooltipText.text = ""
    }
}
```

## Использование в Qt Widgets приложении

### Подключение через QQuickWidget

```cpp
#include <QQuickWidget>
#include <QQmlContext>
#include "mapdata.h"

// В конструкторе вашего виджета:
QQuickWidget *quickWidget = new QQuickWidget(this);

// Создаем backend
MapData *mapData = new MapData(this);
mapData->loadGeoJSON("data/rus_simple_highcharts.geo.json");

// Устанавливаем контекст
quickWidget->rootContext()->setContextProperty("mapData", mapData);

// Загружаем компонент
quickWidget->setSource(QUrl("qrc:/MapComponent.qml"));
quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

// Добавляем в layout
layout->addWidget(quickWidget);
```

### Обработка событий от компонента

```cpp
// Получаем root объект
QObject *rootObject = quickWidget->rootObject();

// Подключаем сигналы
connect(rootObject, SIGNAL(regionClicked(QString, QString)),
        this, SLOT(onRegionClicked(QString, QString)));

connect(rootObject, SIGNAL(regionHovered(QString, QString)),
        this, SLOT(onRegionHovered(QString, QString)));

// Слоты для обработки
void MyWidget::onRegionClicked(const QString &regionId, const QString &regionName)
{
    qDebug() << "Клик по региону:" << regionName;
    // Ваша логика
}
```

### Вызов методов компонента из C++

```cpp
// Перерисовать карту
QMetaObject::invokeMethod(rootObject, "repaint");

// Очистить выделение
QMetaObject::invokeMethod(rootObject, "clearSelection");

// Изменить свойство
rootObject->setProperty("selectedRegion", "spb");
```

## API компонента

### Свойства

| Свойство | Тип | По умолчанию | Описание |
|----------|-----|--------------|----------|
| `mapData` | var | null | Объект MapData с данными карты |
| `selectedRegion` | string | "" | ID выбранного региона |
| `canvasWidth` | int | 1200 | Ширина canvas для рисования |
| `canvasHeight` | int | 1000 | Высота canvas для рисования |
| `backgroundColor` | color | "#ffffff" | Цвет фона |
| `borderColor` | color | "#bdc3c7" | Цвет границы компонента |
| `borderWidth` | int | 1 | Толщина границы компонента |
| `defaultRegionColor` | color | "#e0e0e0" | Цвет регионов по умолчанию |
| `selectedRegionColor` | color | "#e74c3c" | Цвет выбранного региона |
| `strokeColor` | color | "#34495e" | Цвет контуров регионов |
| `strokeWidth` | real | 1 | Толщина контуров |

### Сигналы

| Сигнал | Параметры | Описание |
|--------|-----------|----------|
| `regionClicked` | `string regionId`, `string regionName` | Испускается при клике на регион |
| `regionHovered` | `string regionId`, `string regionName` | Испускается при наведении на регион |
| `regionExited` | - | Испускается когда курсор покидает регион |

### Методы

| Метод | Описание |
|-------|----------|
| `repaint()` | Принудительная перерисовка карты |
| `clearSelection()` | Сброс выделения региона |

## Пример полного приложения на Qt Widgets

Смотрите файл `widget_map_example.h` для полного примера интеграции с Qt Widgets.

Основные шаги:

1. **Создать QQuickWidget**
2. **Создать и настроить MapData backend**
3. **Установить контекст для QML**
4. **Загрузить MapComponent.qml**
5. **Подключить сигналы для обработки событий**

```cpp
// Минимальный пример
QQuickWidget *mapWidget = new QQuickWidget(this);
MapData *mapData = new MapData(this);

mapData->loadGeoJSON("data/rus_simple_highcharts.geo.json");
mapWidget->rootContext()->setContextProperty("mapData", mapData);
mapWidget->setSource(QUrl("qrc:/MapComponent.qml"));

// Обработка кликов
QObject *root = mapWidget->rootObject();
connect(root, SIGNAL(regionClicked(QString,QString)),
        this, SLOT(handleRegionClick(QString,QString)));
```

## Работа с данными карты (MapData)

### C++ API

```cpp
// Загрузка данных
mapData->loadGeoJSON("path/to/file.json");

// Выделение региона цветом
mapData->highlightRegion("moscow", "#ff0000");

// Установка выбранного региона
mapData->setSelectedRegion("spb");

// Очистка всех выделений
mapData->clearHighlights();

// Получение данных
QVariantList regions = mapData->regions();
QString selected = mapData->selectedRegion();
```

### Подключение к сигналам MapData

```cpp
connect(mapData, &MapData::regionsChanged,
        this, &MyClass::onRegionsLoaded);

connect(mapData, &MapData::selectedRegionChanged,
        this, &MyClass::onSelectionChanged);

connect(mapData, &MapData::regionColorsChanged,
        this, &MyClass::onColorsChanged);
```

## Настройка внешнего вида

### Изменение цветовой схемы

```qml
MapComponent {
    defaultRegionColor: "#f0f0f0"      // Светло-серый фон
    selectedRegionColor: "#3498db"     // Синий для выбранного
    strokeColor: "#2c3e50"             // Темные границы
    strokeWidth: 1.5                    // Толще границы
    backgroundColor: "#ecf0f1"          // Светлый фон
}
```

### Изменение размеров

```qml
MapComponent {
    canvasWidth: 1500
    canvasHeight: 1200
    
    // Или масштабировать под родителя
    anchors.fill: parent
}
```

## Обработка кликов - примеры использования

### Показать информацию о регионе

```qml
MapComponent {
    id: map
    
    onRegionClicked: {
        infoDialog.regionId = regionId
        infoDialog.regionName = regionName
        infoDialog.open()
    }
}

Dialog {
    id: infoDialog
    property string regionId
    property string regionName
    
    title: "Информация о регионе"
    Text {
        text: "Регион: " + infoDialog.regionName + 
              "\nID: " + infoDialog.regionId
    }
}
```

### Выделить регион при клике

```qml
MapComponent {
    id: map
    
    onRegionClicked: {
        // Изменяем цвет региона
        mapData.highlightRegion(regionId, "#e74c3c")
        
        // Обновляем выделение
        selectedRegion = regionId
    }
}
```

### Загрузить данные о регионе с сервера

```cpp
void MyWidget::onRegionClicked(const QString &regionId, const QString &regionName)
{
    qDebug() << "Загружаем данные для региона:" << regionName;
    
    // Создаем запрос к API
    QNetworkRequest request(QUrl("https://api.example.com/regions/" + regionId));
    
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, regionName]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            // Обрабатываем данные
            showRegionInfo(regionName, data);
        }
        reply->deleteLater();
    });
}
```

## Тестовый режим

Если GeoJSON файл не загружается, компонент автоматически создаст тестовые данные с несколькими регионами для демонстрации функциональности.

## Требования

- **Qt 5.12** или выше
- **Qt Quick** модули
- **C++11** или выше
- Модули: `QtQuick`, `QtQuick.Controls`, `QtQml`

## Файлы для добавления в проект

### Обязательные файлы

1. `MapComponent.qml` - компонент карты
2. `mapdata.h` - заголовочный файл backend
3. `mapdata.cpp` - реализация backend
4. `data/rus_simple_highcharts.geo.json` - данные карты

### Дополнительно для примеров

5. `map.qml` - пример QML приложения
6. `widget_map_example.h` - пример Qt Widgets
7. `main.cpp` - точка входа

### Обновление .pro файла

```qmake
QT += core qml quick widgets

SOURCES += \
    main.cpp \
    mapdata.cpp

HEADERS += \
    mapdata.h

RESOURCES += \
    resources.qrc
```

### Обновление resources.qrc

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource>
        <file>MapComponent.qml</file>
        <file>map.qml</file>
    </qresource>
    <qresource prefix="/data">
        <file>data/rus_simple_highcharts.geo.json</file>
    </qresource>
</RCC>
```

## Отладка

### Включение консольных сообщений

Компонент выводит отладочную информацию в консоль:

```
=== onPaint вызван ===
Canvas размер: 1200 x 1000
Регионов: 85
Первый регион: Москва и Московская область
Нарисовано путей: 85
```

### Проверка загрузки данных

```qml
Component.onCompleted: {
    console.log("MapComponent готов")
    console.log("Регионов загружено:", mapData ? mapData.regions.length : 0)
}
```

### Визуальная отладка

Если регионы не отображаются:

1. Проверьте, что `mapData` установлен
2. Проверьте загрузку GeoJSON файла
3. Проверьте размеры canvas (`canvasWidth`, `canvasHeight`)
4. Убедитесь, что координаты нормализованы правильно

## Производительность

- Компонент оптимизирован для отображения до 100 регионов
- Использует алгоритм ray-casting для определения кликов
- Перерисовка происходит только при изменении данных

## Лицензия

Проект создан в образовательных целях. Свободно используйте и модифицируйте код.

## Поддержка

При возникновении проблем проверьте:

1. Версию Qt (требуется 5.12+)
2. Наличие всех необходимых модулей
3. Правильность путей к ресурсам
4. Корректность формата GeoJSON данных
