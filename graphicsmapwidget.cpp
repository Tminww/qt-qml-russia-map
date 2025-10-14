
#include "graphicsmapwidget.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>

GraphicsMapWidget::GraphicsMapWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
    createMockRegions();
    populateTree();
}

void GraphicsMapWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // Левая панель
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);

    QLabel *treeLabel = new QLabel("Регионы России");
    treeLabel->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    leftLayout->addWidget(treeLabel);

    treeWidget = new QTreeWidget();
    treeWidget->setHeaderLabel("Федеральные округа");
    treeWidget->setMinimumWidth(250);
    treeWidget->setStyleSheet("QTreeWidget { font-size: 12px; }");
    leftLayout->addWidget(treeWidget);

    QPushButton *clearBtn = new QPushButton("Сбросить выделение");
    leftLayout->addWidget(clearBtn);
    connect(clearBtn, &QPushButton::clicked, this, &GraphicsMapWidget::clearSelection);

    // Правая панель
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);

    infoLabel = new QLabel("Кликните на регион или выберите в дереве");
    infoLabel->setStyleSheet(
        "background-color: #E8F4F8; "
        "padding: 15px; "
        "border-radius: 5px; "
        "font-size: 13px;");
    infoLabel->setWordWrap(true);
    rightLayout->addWidget(infoLabel);

    scene = new QGraphicsScene();
    scene->setBackgroundBrush(QBrush(QColor(240, 248, 255)));

    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    rightLayout->addWidget(view);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    mainLayout->addWidget(splitter);

    connect(treeWidget, &QTreeWidget::itemClicked,
            this, &GraphicsMapWidget::onTreeItemClicked);
}

void GraphicsMapWidget::createMockRegions()
{
    QList<QPair<QString, QRectF>> regions = {
        {"Moscow", QRectF(100, 200, 80, 60)},
        {"SPb", QRectF(20, 100, 70, 70)},
        {"Novosibirsk", QRectF(400, 250, 90, 70)},
        {"Ekaterinburg", QRectF(300, 180, 80, 80)},
        {"Kazan", QRectF(200, 230, 70, 60)},
        {"Nizhny", QRectF(180, 200, 60, 50)}};

    QMap<QString, QString> regionNames = {
        {"Moscow", "Москва"},
        {"SPb", "Санкт-Петербург"},
        {"Novosibirsk", "Новосибирская обл."},
        {"Ekaterinburg", "Свердловская обл."},
        {"Kazan", "Татарстан"},
        {"Nizhny", "Нижегородская обл."}};

    QList<QColor> colors = {
        QColor(255, 200, 200), QColor(200, 255, 200),
        QColor(200, 200, 255), QColor(255, 255, 200),
        QColor(255, 200, 255), QColor(200, 255, 255)};

    int colorIndex = 0;
    for (const auto &pair : regions)
    {
        QString code = pair.first;
        QRectF rect = pair.second;
        QColor color = colors[colorIndex++ % colors.size()];

        RegionItem *item = new RegionItem(code, regionNames[code], rect, color);
        scene->addItem(item);
        regionItems[code] = item;

        RegionData data(regionNames[code], code);
        data.bounds = rect;
        data.color = color;
        regionData[code] = data;

        QGraphicsTextItem *text = scene->addText(regionNames[code]);
        text->setDefaultTextColor(Qt::darkGray);
        text->setFont(QFont("Arial", 9, QFont::Bold));
        text->setPos(rect.center() - QPointF(text->boundingRect().width() / 2,
                                             text->boundingRect().height() / 2));
    }

    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-50, -50, 50, 50));
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void GraphicsMapWidget::populateTree()
{
    treeWidget->clear();

    QTreeWidgetItem *centralItem = new QTreeWidgetItem(treeWidget);
    centralItem->setText(0, "Центральный ФО");
    centralItem->setExpanded(true);
    QTreeWidgetItem *moscowItem = new QTreeWidgetItem(centralItem);
    moscowItem->setText(0, "Москва");
    moscowItem->setData(0, Qt::UserRole, "Moscow");

    QTreeWidgetItem *volga = new QTreeWidgetItem(treeWidget);
    volga->setText(0, "Приволжский ФО");
    volga->setExpanded(true);
    QTreeWidgetItem *kazanItem = new QTreeWidgetItem(volga);
    kazanItem->setText(0, "Республика Татарстан");
    kazanItem->setData(0, Qt::UserRole, "Kazan");
    QTreeWidgetItem *nizhnyItem = new QTreeWidgetItem(volga);
    nizhnyItem->setText(0, "Нижегородская область");
    nizhnyItem->setData(0, Qt::UserRole, "Nizhny");

    QTreeWidgetItem *nw = new QTreeWidgetItem(treeWidget);
    nw->setText(0, "Северо-Западный ФО");
    nw->setExpanded(true);
    QTreeWidgetItem *spbItem = new QTreeWidgetItem(nw);
    spbItem->setText(0, "Санкт-Петербург");
    spbItem->setData(0, Qt::UserRole, "SPb");

    QTreeWidgetItem *ural = new QTreeWidgetItem(treeWidget);
    ural->setText(0, "Уральский ФО");
    ural->setExpanded(true);
    QTreeWidgetItem *ekbItem = new QTreeWidgetItem(ural);
    ekbItem->setText(0, "Свердловская область");
    ekbItem->setData(0, Qt::UserRole, "Ekaterinburg");

    QTreeWidgetItem *siberia = new QTreeWidgetItem(treeWidget);
    siberia->setText(0, "Сибирский ФО");
    siberia->setExpanded(true);
    QTreeWidgetItem *nvsbItem = new QTreeWidgetItem(siberia);
    nvsbItem->setText(0, "Новосибирская область");
    nvsbItem->setData(0, Qt::UserRole, "Novosibirsk");
}

void GraphicsMapWidget::onTreeItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString regionCode = item->data(0, Qt::UserRole).toString();
    if (!regionCode.isEmpty() && regionItems.contains(regionCode))
    {
        selectRegion(regionCode);
    }
}

void GraphicsMapWidget::selectRegion(const QString &regionCode)
{
    for (auto it = regionItems.begin(); it != regionItems.end(); ++it)
    {
        it.value()->setHighlight(false);
    }

    if (regionItems.contains(regionCode))
    {
        RegionItem *item = regionItems[regionCode];
        item->setHighlight(true);

        view->fitInView(item->boundingRect().adjusted(-20, -20, 20, 20),
                        Qt::KeepAspectRatio);

        if (regionData.contains(regionCode))
        {
            infoLabel->setText(QString(
                                   "<b>Выбран регион:</b> %1<br>"
                                   "<b>Код:</b> %2<br>"
                                   "<b>Статус:</b> Выделен")
                                   .arg(regionData[regionCode].name)
                                   .arg(regionCode));
        }
    }
}

void GraphicsMapWidget::clearSelection()
{
    for (auto it = regionItems.begin(); it != regionItems.end(); ++it)
    {
        it.value()->setHighlight(false);
    }
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    infoLabel->setText("Выделение снято. Кликните на регион или выберите в дереве");
}
