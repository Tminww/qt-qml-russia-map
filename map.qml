import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ApplicationWindow {
    id: root
    width: 1400
    height: 1000
    visible: true
    title: "Карта России"
    
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
                    text: "Перерисовать"
                    onClicked: mapCanvas.requestPaint()
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
                color: "#ffffff"
                border.color: "#bdc3c7"
                border.width: 1
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true
                    
                    Canvas {
                        id: mapCanvas
                        width: 1200
                        height: 1000
                        
                        onPaint: {
                            console.log("=== onPaint вызван ===")
                            var ctx = getContext("2d")
                            
                            // Заливаем фон для теста
                            ctx.fillStyle = "#ffffff"
                            ctx.fillRect(0, 0, width, height)
                            
                            console.log("Canvas размер:", width, "x", height)
                            console.log("mapData.regions:", mapData.regions)
                            console.log("Регионов:", mapData.regions ? mapData.regions.length : 0)
                            
                            if (!mapData.regions || mapData.regions.length === 0) {
                                ctx.fillStyle = "#ff0000"
                                ctx.font = "20px Arial"
                                ctx.fillText("Нет данных для отображения", 50, 50)
                                console.log("НЕТ ДАННЫХ!")
                                return
                            }
                            
                            
                            // Рисуем каждый регион
                            var drawnCount = 0
                            for (var i = 0; i < mapData.regions.length; i++) {
                                var region = mapData.regions[i]
                                var paths = region.paths || []
                                
                                if (i === 0) {
                                    console.log("Первый регион:", region.name)
                                    console.log("Paths:", paths)
                                    console.log("Первый path:", paths.length > 0 ? paths[0] : "нет")
                                }
                                
                                // Определяем цвет
                                var fillColor = "#e0e0e0"
                                if (mapData.selectedRegion === region.id) {
                                    fillColor = "#e74c3c"
                                } else if (mapData.regionColors[region.id]) {
                                    fillColor = mapData.regionColors[region.id]
                                }
                                
                                // Рисуем все пути региона
                                for (var j = 0; j < paths.length; j++) {
                                    drawPath(ctx, paths[j], fillColor)
                                    drawnCount++
                                }
                            }
                            
                            console.log("Нарисовано путей:", drawnCount)
                        }
                        
                        function drawPath(ctx, pathString, fillColor) {
                            if (!pathString) {
                                console.log("Пустой pathString!")
                                return
                            }
                            
                            ctx.fillStyle = fillColor
                            ctx.strokeStyle = "#34495e"
                            ctx.lineWidth = 1
                            
                            ctx.beginPath()
                            
                            // Парсим SVG путь вручную
                            var tokens = pathString.replace(/([MLZ])/g, "|$1 ").split("|")
                            var pointCount = 0
                            
                            for (var i = 0; i < tokens.length; i++) {
                                var token = tokens[i].trim()
                                if (!token) continue
                                
                                var parts = token.split(/\s+/)
                                var cmd = parts[0]
                                
                                if (cmd === 'M' && parts.length >= 3) {
                                    var x = parseFloat(parts[1])
                                    var y = parseFloat(parts[2])
                                    ctx.moveTo(x, y)
                                    pointCount++
                                    
                                    if (i === 0) {
                                        console.log("Первая точка: M", x, y)
                                    }
                                } else if (cmd === 'L' && parts.length >= 3) {
                                    var x = parseFloat(parts[1])
                                    var y = parseFloat(parts[2])
                                    ctx.lineTo(x, y)
                                    pointCount++
                                } else if (cmd === 'Z') {
                                    ctx.closePath()
                                }
                            }
                            
                            if (pointCount > 0) {
                                ctx.fill()
                                ctx.stroke()
                            } else {
                                console.log("Не найдено точек в пути!")
                            }
                        }
                        
                        Connections {
                            target: mapData
                            function onRegionsChanged() {
                                mapCanvas.requestPaint()
                            }
                            function onRegionColorsChanged() {
                                mapCanvas.requestPaint()
                            }
                            function onSelectedRegionChanged() {
                                mapCanvas.requestPaint()
                            }
                        }
                        
                        Component.onCompleted: {
                            console.log("Canvas готов, перерисовываем...")
                            requestPaint()
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