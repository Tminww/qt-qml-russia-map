#ifndef MAPDATA_H
#define MAPDATA_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>

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


signals:
    void regionsChanged();
    void regionStatusChanged(const QString &regionId, const QString &regionStatus);
    void selectedRegionChanged(const QString &regionId);

private:
    void parseGeoJSON(const QJsonDocument &doc);
    QVariantList coordinatesToPath(const QJsonArray &coordinates,
                                   double minX, double maxX,
                                   double minY, double maxY);
    void findBounds(const QJsonArray &coordinates,
                    double &minX, double &maxX,
                    double &minY, double &maxY);

    QVariantList m_regions;
    QString m_selectedRegion;
};

#endif // MAPDATA_H
