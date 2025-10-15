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
    Q_PROPERTY(QVariantMap regionColors READ regionColors NOTIFY regionColorsChanged)
    Q_PROPERTY(QString selectedRegion READ selectedRegion WRITE setSelectedRegion NOTIFY selectedRegionChanged)

public:
    explicit MapData(QObject *parent = nullptr);

    QVariantList regions() const { return m_regions; }
    QVariantMap regionColors() const { return m_regionColors; }
    QString selectedRegion() const { return m_selectedRegion; }
    void setSelectedRegion(const QString &regionId);

    Q_INVOKABLE void loadGeoJSON(const QString &filePath);
    Q_INVOKABLE void highlightRegion(const QString &regionId, const QString &color);
    Q_INVOKABLE void clearHighlights();

signals:
    void regionsChanged();
    void regionColorsChanged();
    void selectedRegionChanged();

private:
    void parseGeoJSON(const QJsonDocument &doc);
    QVariantList coordinatesToPath(const QJsonArray &coordinates,
                                   double minX, double maxX,
                                   double minY, double maxY);
    void findBounds(const QJsonArray &coordinates,
                    double &minX, double &maxX,
                    double &minY, double &maxY);

    QVariantList m_regions;
    QVariantMap m_regionColors;
    QString m_selectedRegion;
};

#endif // MAPDATA_H