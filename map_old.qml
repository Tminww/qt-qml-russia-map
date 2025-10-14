import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Shapes 1.12

ApplicationWindow {
    id: root
    width: 1400
    height: 1000
    visible: true
    title: "Карта России"
    
    Component {
        id: shapePathComponent
        ShapePath {
            property string regionId: ""
            property string regionName: ""
            
            strokeWidth: 1
            strokeColor: "#34495e"
            fillColor: {
                if (mapData.selectedRegion === regionId) {
                    return "#e74c3c"
                }
                return mapData.regionColors[regionId] || "#e0e0e0"
            }
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        
        // Заголовок
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#2c3e50"
            
            RowLayout {
                anchors.centerIn: parent
                spacing: 20
                
                Text {
                    text: "Карта России"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                }
                
                Text {
                    text: "Регионов: " + (mapData.regions ? mapData.regions.length : 0)
                    color: "#ecf0f1"
                    font.pixelSize: 12
                }
                
                Button {
                    text: "Сбросить"
                    onClicked: mapData.clearHighlights()
                }
                
                Button {
                    text: "Случайный цвет"
                    onClicked: {
                        var colors = ["#ff6b6b", "#4ecdc4", "#45b7d1", "#96ceb4", "#feca57", "#ff9ff3"]
                        if (mapData.regions && mapData.regions.length > 0) {
                            var idx = Math.floor(Math.random() * mapData.regions.length)
                            var region = mapData.regions[idx]
                            var color = colors[Math.floor(Math.random() * colors.length)]
                            mapData.highlightRegion(region.id, color)
                        }
                    }
                }
            }
        }
        
        // Основной контент
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            
            // Левая панель
            Rectangle {
                Layout.preferredWidth: 300
                Layout.fillHeight: true
                color: "#ecf0f1"
                border.color: "#bdc3c7"
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    
                    Text {
                        text: "Регионы России"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#2c3e50"
                    }
                    
                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        
                        ListView {
                            id: regionList
                            model: mapData.regions || []
                            clip: true
                            
                            delegate: Rectangle {
                                width: regionList.width
                                height: 40
                                color: mouseArea.containsMouse ? "#3498db" : 
                                       (mapData.selectedRegion === modelData.id ? "#2ecc71" : "transparent")
                                
                                Text {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 10
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width - 20
                                    text: modelData.name || ""
                                    color: (mouseArea.containsMouse || mapData.selectedRegion === modelData.id) ? "white" : "#2c3e50"
                                    elide: Text.ElideRight
                                }
                                
                                MouseArea {
                                    id: mouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        mapData.selectedRegion = modelData.id || ""
                                        console.log("Выбран:", modelData.name)
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // Правая панель с картой
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#f8f9fa"
                border.color: "#bdc3c7"
                border.width: 1
                
                ScrollView {
                    id: scrollView
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true
                    
                    Item {
                        width: Math.max(scrollView.width - 20, 1000)
                        height: Math.max(scrollView.height - 20, 800)
                        
                        Shape {
                            id: mapShape
                            width: 1000
                            height: 800
                            anchors.centerIn: parent
                            
                            // ShapePath элементы будут добавлены динамически
                        }
                        
                        Component.onCompleted: {
                            createRegions()
                        }
                        
                        function createRegions() {
                            console.log("Создаем регионы на карте...")
                            
                            if (!mapData.regions || mapData.regions.length === 0) {
                                console.log("Нет данных для отображения")
                                return
                            }
                            
                            var count = 0
                            for (var i = 0; i < mapData.regions.length; i++) {
                                var region = mapData.regions[i]
                                var paths = region.paths || []
                                
                                for (var j = 0; j < paths.length; j++) {
                                    var shapePath = shapePathComponent.createObject(mapShape, {
                                        regionId: region.id,
                                        regionName: region.name
                                    })
                                    
                                    if (shapePath) {
                                        var pathSvg = Qt.createQmlObject(
                                            'import QtQuick.Shapes 1.12; PathSvg { path: "' + paths[j] + '" }',
                                            shapePath,
                                            "dynamicPath"
                                        )
                                        count++
                                    }
                                }
                            }
                            
                            console.log("Создано путей:", count)
                        }
                        
                        Connections {
                            target: mapData
                            function onRegionsChanged() {
                                createRegions()
                            }
                        }
                    }
                }
                
                // Информация о выбранном регионе
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 40
                    color: "#34495e"
                    opacity: 0.9
                    visible: mapData.selectedRegion !== ""
                    
                    Text {
                        anchors.centerIn: parent
                        color: "white"
                        font.pixelSize: 14
                        text: {
                            if (!mapData.selectedRegion) return ""
                            for (var i = 0; i < mapData.regions.length; i++) {
                                if (mapData.regions[i].id === mapData.selectedRegion) {
                                    return "Выбран: " + mapData.regions[i].name
                                }
                            }
                            return ""
                        }
                    }
                }
            }
        }
    }
    
    Component.onCompleted: {
        console.log("QML загружен, регионов:", mapData.regions ? mapData.regions.length : 0)
    }
}