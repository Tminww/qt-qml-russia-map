#include "mapdata.h"
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>

MapData::MapData(QObject *parent)
    : QObject(parent), m_selectedRegion("")
{

}

void MapData::loadGeoJSON(const QString &filePath)
{
    QFile resourceFile(":/data/" + filePath);
    if (!resourceFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Не удалось открыть файл из ресурсов:" << ":/data/" + filePath;
        return;
    }

    QByteArray data = resourceFile.readAll();
    resourceFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
    {
        qDebug() << "Ошибка парсинга JSON из ресурсов";
        return;
    }

    parseGeoJSON(doc);
}

void MapData::parseGeoJSON(const QJsonDocument &doc)
{
    QJsonObject root = doc.object();
    QJsonArray features = root["features"].toArray();

    m_regions.clear();

    qDebug() << "Начинаем парсинг GeoJSON, количество features:" << features.size();

    // Сначала найдем границы координат для нормализации
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    // Первый проход - находим границы
    for (const QJsonValue &value : features)
    {
        QJsonObject feature = value.toObject();
        QJsonObject geometry = feature["geometry"].toObject();
        QJsonArray coordinates = geometry["coordinates"].toArray();

        findBounds(coordinates, minX, maxX, minY, maxY);
    }

    qDebug() << "Границы координат: X[" << minX << "," << maxX << "] Y[" << minY << "," << maxY << "]";



    // Второй проход - создаем регионы
    for (const QJsonValue &value : features)
    {
        QJsonObject feature = value.toObject();
        QJsonObject properties = feature["properties"].toObject();
        QJsonObject geometry = feature["geometry"].toObject();

        QString name = properties["name"].toString();
        QString hcKey = properties["hc-key"].toString();
        QString postalCode = properties["postal-code"].toString();

        if (name.isEmpty())
        {
            name = hcKey;
        }

        if (hcKey.isEmpty())
        {
            qDebug() << "Пропущен регион без hc-key, name:" << name;
            continue;
        }

        // Преобразуем координаты в пути для QML
        QVariantList paths = coordinatesToPath(geometry["coordinates"].toArray(),
                                               minX, maxX, minY, maxY);

        if (paths.isEmpty())
        {
            qDebug() << "Пустой path для региона:" << name;
            continue;
        }

        QString status = "default";

        if (hcKey == "10312" || hcKey == "10401" || hcKey == "10404" || hcKey == "10201"){
            status = "warning";
        } else if (hcKey == "10202" || hcKey == "10505" || hcKey == "10205" || hcKey == "11103"){
            status = "danger";
        }


        QVariantMap region;
        region["name"] = name;
        region["id"] = hcKey;
        region["postal-code"] = postalCode;
        region["paths"] = paths;
        region["status"] = status;

        m_regions.append(region);



    }

    qDebug() << "Всего загружено регионов:" << m_regions.size();

    emit regionsChanged();

}

void MapData::findBounds(const QJsonArray &coordinates, double &minX, double &maxX,
                         double &minY, double &maxY)
{
    for (const QJsonValue &item : coordinates)
    {
        if (item.isArray())
        {
            QJsonArray arr = item.toArray();

            // Проверяем, это координата или массив координат
            if (arr.size() >= 2 && arr[0].isDouble() && arr[1].isDouble())
            {
                double x = arr[0].toDouble();
                double y = arr[1].toDouble();

                minX = qMin(minX, x);
                maxX = qMax(maxX, x);
                minY = qMin(minY, y);
                maxY = qMax(maxY, y);
            }
            else
            {
                // Рекурсивно обрабатываем вложенные массивы
                findBounds(arr, minX, maxX, minY, maxY);
            }
        }
    }
}

QVariantList MapData::coordinatesToPath(const QJsonArray &coordinates,
                                        double minX, double maxX,
                                        double minY, double maxY)
{
    QVariantList paths;
    // Размеры целевого canvas
    const double targetWidth = 1000.0;
    const double targetHeight = 700.0;
    const double padding = 0.0;

    double rangeX = maxX - minX;
    double rangeY = maxY - minY;


    if (rangeX == 0 || rangeY == 0)
    {
        qDebug() << "Некорректный диапазон координат";
        return paths;
    }

    // Вычисляем масштаб с сохранением пропорций
    double scaleX = (targetWidth - 2 * padding) / rangeX;
    double scaleY = (targetHeight - 2 * padding) / rangeY;
    double scale = qMin(scaleX, scaleY);

    // Обрабатываем MultiPolygon
    for (const QJsonValue &polygon : coordinates)
    {
        if (!polygon.isArray())
            continue;

        QJsonArray polygonArray = polygon.toArray();

        for (const QJsonValue &ring : polygonArray)
        {
            if (!ring.isArray())
                continue;

            QJsonArray ringArray = ring.toArray();
            QStringList pathCommands;

            bool first = true;
            for (const QJsonValue &coord : ringArray)
            {
                if (!coord.isArray())
                    continue;

                QJsonArray coordArray = coord.toArray();
                if (coordArray.size() < 2)
                    continue;

                double x = coordArray[0].toDouble();
                double y = coordArray[1].toDouble();

                // Нормализуем и масштабируем координаты
                double normalizedX = (x - minX) * scale + padding ;
                // Инвертируем Y, так как SVG координаты идут сверху вниз
                double normalizedY = targetHeight - ((y - minY) * scale + padding);

                if (first)
                {
                    pathCommands.append(QString("M %1 %2").arg(normalizedX, 0, 'f', 2).arg(normalizedY, 0, 'f', 2));
                    first = false;
                }
                else
                {
                    pathCommands.append(QString("L %1 %2").arg(normalizedX, 0, 'f', 2).arg(normalizedY, 0, 'f', 2));
                }
            }

            if (!pathCommands.isEmpty())
            {
                pathCommands.append("Z"); // Закрываем путь
                paths.append(pathCommands.join(" "));
            }
        }
    }

    return paths;
}

void MapData::setSelectedRegion(const QString &regionId)
{
    if (m_selectedRegion != regionId)
    {
        m_selectedRegion = regionId;
        // Нужно регион добавлять в конец, чтобы он отрисовывалс поверх всех
        emit selectedRegionChanged();
        qDebug() << "Выбран регион:" << regionId;
    }
}
