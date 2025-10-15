#ifndef WIDGET_MAP_EXAMPLE_H
#define WIDGET_MAP_EXAMPLE_H

#include <QWidget>
#include <QQuickWidget>
#include <QQuickItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QQmlContext>
#include <QRandomGenerator>
#include "MapData.h"

class WidgetMapExample : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetMapExample(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setupUI();
        setupConnections();
    }

private:
    void setupUI()
    {
        // Основной layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Верхняя панель с кнопками
        QHBoxLayout *topLayout = new QHBoxLayout();
        QPushButton *refreshBtn = new QPushButton("Перерисовать", this);
        QPushButton *clearBtn = new QPushButton("Сбросить", this);
        QPushButton *randomColorBtn = new QPushButton("Случайный цвет", this);

        topLayout->addWidget(refreshBtn);
        topLayout->addWidget(clearBtn);
        topLayout->addWidget(randomColorBtn);
        topLayout->addStretch();
        mainLayout->addLayout(topLayout);

        // QQuickWidget для отображения QML карты
        m_quickWidget = new QQuickWidget(this);
        m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
        m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // Создаём backend данных
        m_mapData = new MapData(this);
        m_mapData->loadGeoJSON("data/rus_simple_highcharts.geo.json");

        // Передаём данные в QML
        m_quickWidget->rootContext()->setContextProperty("mapData", m_mapData);

        // Загружаем сам компонент карты
        m_quickWidget->setSource(QUrl("qrc:/MapComponent.qml"));

        mainLayout->addWidget(m_quickWidget);

        // Статусная строка
        m_statusLabel = new QLabel("Готово к работе", this);
        m_statusLabel->setStyleSheet("QLabel { color: white; background-color: #34495e; padding: 5px; }");
        mainLayout->addWidget(m_statusLabel);

        // Подключаем кнопки
        connect(refreshBtn, &QPushButton::clicked, this, &WidgetMapExample::onRefreshClicked);
        connect(clearBtn, &QPushButton::clicked, this, &WidgetMapExample::onClearClicked);
        connect(randomColorBtn, &QPushButton::clicked, this, &WidgetMapExample::onRandomColorClicked);
    }

    void setupConnections()
    {
        // Получаем сам корневой объект QML
        QQuickItem *rootItem = m_quickWidget->rootObject();
        if (!rootItem)
            return;

        // Подключаем сигналы из QML (regionClicked / regionHovered / regionExited)
        connect(rootItem, SIGNAL(regionClicked(QString, QString)),
                this, SLOT(onRegionClicked(QString, QString)));

        connect(rootItem, SIGNAL(regionHovered(QString, QString)),
                this, SLOT(onRegionHovered(QString, QString)));

        connect(rootItem, SIGNAL(regionExited()),
                this, SLOT(onRegionExited()));
    }

private slots:
    void onRefreshClicked()
    {
        if (auto *rootItem = m_quickWidget->rootObject())
            QMetaObject::invokeMethod(rootItem, "repaint");
    }

    void onClearClicked()
    {
        m_mapData->clearHighlights();
        if (auto *rootItem = m_quickWidget->rootObject())
            QMetaObject::invokeMethod(rootItem, "clearSelection");

        m_statusLabel->setText("Выделение сброшено");
    }

    void onRandomColorClicked()
    {
        QStringList colors = {"#ff6b6b", "#4ecdc4", "#45b7d1", "#96ceb4", "#feca57", "#ff9ff3"};

        QVariantList regions = m_mapData->regions();
        if (!regions.isEmpty())
        {
            int idx = QRandomGenerator::global()->bounded(regions.size());
            QVariantMap region = regions[idx].toMap();
            QString regionId = region["id"].toString();
            QString color = colors[QRandomGenerator::global()->bounded(colors.size())];

            m_mapData->highlightRegion(regionId, color);
            m_statusLabel->setText(QString("Выделен регион: %1").arg(region["name"].toString()));
        }
    }

    void onRegionClicked(const QString &regionId, const QString &regionName)
    {
        qDebug() << "Клик по региону:" << regionId << regionName;
        m_mapData->setSelectedRegion(regionId);
        m_statusLabel->setText(QString("Выбран: %1").arg(regionName));
    }

    void onRegionHovered(const QString &regionId, const QString &regionName)
    {
        m_statusLabel->setText(QString("Наведение: %1").arg(regionName));
    }

    void onRegionExited()
    {
        if (!m_mapData->selectedRegion().isEmpty())
        {
            QVariantList regions = m_mapData->regions();
            for (const QVariant &v : regions)
            {
                QVariantMap region = v.toMap();
                if (region["id"].toString() == m_mapData->selectedRegion())
                {
                    m_statusLabel->setText(QString("Выбран: %1").arg(region["name"].toString()));
                    return;
                }
            }
        }
        m_statusLabel->setText("Готово к работе");
    }

private:
    QQuickWidget *m_quickWidget;
    MapData *m_mapData;
    QLabel *m_statusLabel;
};

#endif // WIDGET_MAP_EXAMPLE_H
