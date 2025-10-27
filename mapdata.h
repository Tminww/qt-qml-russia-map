#ifndef MAPDATA_H
#define MAPDATA_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QVector>


class MapData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList regions READ regions NOTIFY regionsChanged)
    Q_PROPERTY(QString selectedRegion READ selectedRegion WRITE setSelectedRegion NOTIFY selectedRegionChanged)

public:
    explicit MapData(QObject *parent = nullptr);

    QVariantList regions() const { return m_regions; }
    QString selectedRegion() const { return m_selectedRegion; }
    void setSelectedRegion(const QString &regionId);

    Q_INVOKABLE void loadGeoJSON(const QString &filePath);

    // Методы для управления регионами
    Q_INVOKABLE QVariantMap getRegionById(const QString &regionId) const;
    Q_INVOKABLE void updateRegionStatus(const QString &regionId, const QString &status);
    Q_INVOKABLE void clearSelection();

    // Внутренний метод для вызова из QML
    Q_INVOKABLE void notifyRegionClicked(const QString &regionId, const QString &regionName);

    // НОВЫЕ МЕТОДЫ ДЛЯ ОПТИМИЗАЦИИ - вычисления в C++
    Q_INVOKABLE QVariantMap getRegionAtPoint(qreal x, qreal y, qreal scale, qreal offsetX, qreal offsetY) const;
    Q_INVOKABLE void prepareRegionGeometry();

signals:
    void regionsChanged();
    void regionStatusChanged(const QString &regionId, const QString &status);
    void selectedRegionChanged(const QString &regionId);

    // Новые сигналы для обработки событий
    void regionClicked(const QString &regionId, const QString &regionName);

private:
    struct RegionGeometry {
        QString id;
        QString name;
        QString status;
        QString postalCode;
        QRectF boundingBox;
        QVector<QVector<QPointF>> polygons; // Список полигонов (каждый полигон = список точек)
    };

    void parseGeoJSON(const QJsonDocument &doc);
    QVariantList coordinatesToPath(const QJsonArray &coordinates,
                                   double minX, double maxX,
                                   double minY, double maxY);
    void findBounds(const QJsonArray &coordinates,
                    double &minX, double &maxX,
                    double &minY, double &maxY);

    // Вспомогательные методы для геометрии
    bool isPointInPolygon(const QPointF &point, const QVector<QPointF> &polygon) const;
    QRectF calculateBoundingBox(const QVector<QVector<QPointF>> &polygons) const;

    QVariantList m_regions;
    QString m_selectedRegion;
    QVector<RegionGeometry> m_regionGeometry; // Кеш геометрии для быстрого поиска
};

#endif // MAPDATA_H

