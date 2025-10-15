import QtQuick 2.12
import QtQuick.Controls 2.12

// Отдельный компонент карты для переиспользования
Item {
    id: mapComponent
    
    // Публичные свойства
    property var mapData: null
    property string selectedRegion: ""
    property int canvasWidth: 1200
    property int canvasHeight: 1000
    property color backgroundColor: "#ffffff"
    property color borderColor: "#bdc3c7"
    property int borderWidth: 1
    property color defaultRegionColor: "#e0e0e0"
    property color selectedRegionColor: "#e74c3c"
    property color strokeColor: "#34495e"
    property real strokeWidth: 1
    
    // Сигналы
    signal regionClicked(string regionId, string regionName)
    signal regionHovered(string regionId, string regionName)
    signal regionExited()
    
    // Основной контейнер
    Rectangle {
        anchors.fill: parent
        color: backgroundColor
        border.color: borderColor
        border.width: borderWidth
        
        ScrollView {
            anchors.fill: parent
            anchors.margins: 10
            clip: true
            
            Canvas {
                id: mapCanvas
                width: canvasWidth
                height: canvasHeight

                // Храним информацию о регионах для определения кликов
                property var regionPaths: []
                
                onPaint: {
                    var ctx = getContext("2d")
                    
                    // Очищаем canvas
                    ctx.fillStyle = "#ffffff"
                    ctx.fillRect(0, 0, width, height)
                                        console.log("MapComponent initialized", mapData);

                    
                    if (!mapData || !mapData.regions || mapData.regions.length === 0) {
                        // Показываем сообщение, если нет данных
                        ctx.fillStyle = "#95a5a6"
                        ctx.font = "16px Arial"
                        ctx.fillText("Нет данных для отображения", 50, 50)
                        return
                    }
                    
                    // Очищаем информацию о путях
                    regionPaths = []
                    
                    // Рисуем каждый регион
                    for (var i = 0; i < mapData.regions.length; i++) {
                        var region = mapData.regions[i]
                        var paths = region.paths || []
                        
                        // Определяем цвет
                        var fillColor = defaultRegionColor
                        if (selectedRegion === region.id) {
                            fillColor = selectedRegionColor
                        } else if (mapData.regionColors && mapData.regionColors[region.id]) {
                            fillColor = mapData.regionColors[region.id]
                        }
                        
                        // Рисуем все пути региона и сохраняем их для обработки кликов
                        for (var j = 0; j < paths.length; j++) {
                            var path2D = drawPath(ctx, paths[j], fillColor)
                            if (path2D) {
                                regionPaths.push({
                                    id: region.id,
                                    name: region.name,
                                    path: path2D
                                })
                            }
                        }
                    }
                }
                
                function drawPath(ctx, pathString, fillColor) {
                    if (!pathString) {
                        return null
                    }
                    
                    ctx.fillStyle = fillColor
                    ctx.strokeStyle = strokeColor
                    ctx.lineWidth = strokeWidth
                    
                    ctx.beginPath()
                    
                    // Парсим SVG путь
                    var tokens = pathString.replace(/([MLZ])/g, "|$1 ").split("|")
                    var hasPoints = false
                    
                    for (var i = 0; i < tokens.length; i++) {
                        var token = tokens[i].trim()
                        if (!token) continue
                        
                        var parts = token.split(/\s+/)
                        var cmd = parts[0]
                        
                        if (cmd === 'M' && parts.length >= 3) {
                            var x = parseFloat(parts[1])
                            var y = parseFloat(parts[2])
                            ctx.moveTo(x, y)
                            hasPoints = true
                        } else if (cmd === 'L' && parts.length >= 3) {
                            var x = parseFloat(parts[1])
                            var y = parseFloat(parts[2])
                            ctx.lineTo(x, y)
                            hasPoints = true
                        } else if (cmd === 'Z') {
                            ctx.closePath()
                        }
                    }
                    
                    if (hasPoints) {
                        ctx.fill()
                        ctx.stroke()
                        
                        // Создаем Path2D для проверки кликов (если доступен)
                        // В QML Canvas Path2D может быть недоступен, поэтому возвращаем строку пути
                        return pathString
                    }
                    
                    return null
                }
                
                // Обработка кликов по карте
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    
                    onClicked: {
                        var clickedRegion = mapCanvas.getRegionAtPoint(mouse.x, mouse.y)
                        if (clickedRegion) {
                            mapComponent.selectedRegion = clickedRegion.id
                            mapComponent.regionClicked(clickedRegion.id, clickedRegion.name)
                            mapCanvas.requestPaint()
                        }
                    }
                    
                    onPositionChanged: {
                        var hoveredRegion = mapCanvas.getRegionAtPoint(mouse.x, mouse.y)
                        if (hoveredRegion) {
                            cursorShape = Qt.PointingHandCursor
                            mapComponent.regionHovered(hoveredRegion.id, hoveredRegion.name)
                        } else {
                            cursorShape = Qt.ArrowCursor
                        }
                    }
                    
                    onExited: {
                        cursorShape = Qt.ArrowCursor
                        mapComponent.regionExited()
                    }
                }
                
                // Функция для определения региона по координатам клика
                function getRegionAtPoint(x, y) {
                    // Простая проверка: проходим по всем регионам в обратном порядке
                    // (последние нарисованные сверху)
                    for (var i = regionPaths.length - 1; i >= 0; i--) {
                        var regionInfo = regionPaths[i]
                        
                        // Упрощенная проверка через bounding box
                        // В идеале нужна точная проверка point-in-polygon
                        if (isPointInRegion(x, y, regionInfo.path)) {
                            return regionInfo
                        }
                    }
                    return null
                }
                
                // Упрощенная проверка точки в регионе через парсинг пути
                function isPointInRegion(px, py, pathString) {
                    if (!pathString) return false
                    
                    var tokens = pathString.replace(/([MLZ])/g, "|$1 ").split("|")
                    var points = []
                    
                    for (var i = 0; i < tokens.length; i++) {
                        var token = tokens[i].trim()
                        if (!token) continue
                        
                        var parts = token.split(/\s+/)
                        var cmd = parts[0]
                        
                        if ((cmd === 'M' || cmd === 'L') && parts.length >= 3) {
                            var x = parseFloat(parts[1])
                            var y = parseFloat(parts[2])
                            points.push({x: x, y: y})
                        }
                    }
                    
                    if (points.length < 3) return false
                    
                    // Алгоритм ray casting для проверки точки в полигоне
                    var inside = false
                    for (var i = 0, j = points.length - 1; i < points.length; j = i++) {
                        var xi = points[i].x, yi = points[i].y
                        var xj = points[j].x, yj = points[j].y
                        
                        var intersect = ((yi > py) !== (yj > py))
                            && (px < (xj - xi) * (py - yi) / (yj - yi) + xi)
                        if (intersect) inside = !inside
                    }
                    
                    return inside
                }
                
                // Подключаем обновление при изменении данных
                Connections {
                    target: mapData
                    function onRegionsChanged() {
                        mapCanvas.requestPaint()
                    }
                    function onRegionColorsChanged() {
                        mapCanvas.requestPaint()
                    }
                }
                
                Component.onCompleted: {
                    requestPaint()
                }
            }
        }
    }
    
    // Публичные методы
    function repaint() {
        if (mapCanvas) {
            mapCanvas.requestPaint()
        }
    }
    
    function clearSelection() {
        selectedRegion = ""
        repaint()
    }
    
    // Следим за изменением selectedRegion извне
    onSelectedRegionChanged: {
        repaint()
    }
}