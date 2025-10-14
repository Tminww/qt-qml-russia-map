
#ifndef GRAPHICSMAPWIDGET_H
#define GRAPHICSMAPWIDGET_H

#include <QWidget>
#include <QMap>
#include "regionitem.h"

class QGraphicsView;
class QGraphicsScene;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;

struct RegionData
{
    QString name;
    QString code;
    QColor color;
    QRectF bounds;
    bool highlighted;

    RegionData(const QString &n = "", const QString &c = "")
        : name(n), code(c), color(Qt::lightGray), highlighted(false) {}
};

class GraphicsMapWidget : public QWidget
{
    Q_OBJECT

private:
    QGraphicsView *view;
    QGraphicsScene *scene;
    QTreeWidget *treeWidget;
    QLabel *infoLabel;
    QMap<QString, RegionItem *> regionItems;
    QMap<QString, RegionData> regionData;

    void setupUI();
    void createMockRegions();
    void populateTree();
    void selectRegion(const QString &regionCode);

public:
    explicit GraphicsMapWidget(QWidget *parent = nullptr);

private slots:
    void onTreeItemClicked(QTreeWidgetItem *item, int column);
    void clearSelection();
};

#endif // GRAPHICSMAPWIDGET_H