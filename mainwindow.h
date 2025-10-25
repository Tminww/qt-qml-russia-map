#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MapData; // Forward declaration

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слоты для обработки событий карты
    void onRegionClicked(const QString &regionId, const QString &regionName);
    void onSelectedRegionChanged(const QString &regionId);
    void onRegionStatusChanged(const QString &regionId, const QString &status);
    void onRegionsChanged();

private:
    Ui::MainWindow *ui;

    // Вспомогательные методы
    void connectMapDataSignals(MapData *mapData);
};

#endif // MAINWINDOW_H
