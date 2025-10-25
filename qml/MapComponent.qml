import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import MapData 1.0

Item {
    id: mapComponent

    // Публичные свойства для настройки
    property color backgroundColor: "#f5f5f5"
    property color borderColor: "#bdc3c7"

    // Цвета по умолчанию для регионов
    property color defaultColor: "#5CA8FF"
    property color warningColor: "#FFB84D"
    property color dangerColor: "#FF5C5C"

    // Цвета для активных (выбранных) регионов
    property color defaultActiveColor: "#1E7FFF"
    property color warningActiveColor: "#FF9500"
    property color dangerActiveColor: "#E53935"

    // Цвета обводки
    property color strokeColor: "#ffffff"
    property color activeStrokeColor: "#000000"
    property real strokeWidth: 1
    property real activeStrokeWidth: 2

    // Сигналы для внешнего использования
    signal regionClicked(string regionId, string regionName)
    signal regionHovered(string regionId, string regionName)
    signal regionExited()

    // Таймер для debounce при изменении размера
    Timer {
        id: resizeTimer
        interval: 150
        repeat: false
        onTriggered: {
            console.log("Перерисовка после изменения размера")
            mapCanvas.requestPaint()
        }
    }

    Canvas {
        id: mapCanvas
        anchors.fill: parent

        // Оптимизация рендеринга
        renderStrategy: Canvas.Threaded
        renderTarget: Canvas.FramebufferObject

        // Внутренние свойства для масштабирования
        property real actualScale: 1.0
        property real offsetX: 0
        property real offsetY: 0
        property bool isPainting: false

        onPaint: {
            // Защита от повторного входа
            if (isPainting) {
                console.log("Рендеринг уже выполняется, пропускаем")
                return
            }

            isPainting = true
            console.log("=== Canvas onPaint вызван ===")

            var ctx = getContext("2d")

            if (!ctx) {
                console.error("Не удалось получить контекст 2D!")
                isPainting = false
                return
            }

            // Очищаем canvas
            ctx.clearRect(0, 0, width, height)

            // Проверяем наличие данных
            if (!mapData) {
                console.warn("mapData не определен!")
                drawErrorMessage(ctx, "MapData не инициализирован")
                isPainting = false
                return
            }

            if (!mapData.regions || mapData.regions.length === 0) {
                console.warn("mapData.regions пуст!")
                drawErrorMessage(ctx, "Нет данных для отображения")
                isPainting = false
                return
            }

            console.log("Регионов для отрисовки:", mapData.regions.length)
            console.log("Размер canvas:", width, "x", height)

            // Вычисляем масштаб и смещение для центрирования
            calculateScaleAndOffset()

            // Применяем трансформацию
            ctx.save()
            ctx.translate(offsetX, offsetY)
            ctx.scale(actualScale, actualScale)

            var drawnCount = 0
            var totalPaths = 0

            // Рисуем каждый регион
            for (var i = 0; i < mapData.regions.length; i++) {
                var region = mapData.regions[i]

                if (!region) {
                    console.warn("Регион", i, "не определен")
                    continue
                }

                var paths = region.paths || []
                totalPaths += paths.length

                if (paths.length === 0) {
                    continue
                }

                // Определяем цвет заливки на основе статуса
                var fillColor = defaultColor
                var isSelected = mapData.selectedRegion === region.id

                if (region.status === "warning") {
                    fillColor = isSelected ? warningActiveColor : warningColor
                } else if (region.status === "danger") {
                    fillColor = isSelected ? dangerActiveColor : dangerColor
                } else {
                    fillColor = isSelected ? defaultActiveColor : defaultColor
                }

                // Определяем параметры обводки
                var currentStrokeColor = strokeColor
                var currentStrokeWidth = strokeWidth

                // Рисуем все пути региона
                for (var j = 0; j < paths.length; j++) {
                    var pathDrawn = drawPath(
                        ctx,
                        paths[j],
                        fillColor,
                        currentStrokeWidth,
                        currentStrokeColor
                    )

                    if (pathDrawn) {
                        drawnCount++
                    }
                }
            }

            console.log("Нарисовано путей:", drawnCount, "из", totalPaths)

            // Восстанавливаем контекст
            ctx.restore()

            // Снимаем флаг рендеринга
            isPainting = false
        }

        // Функция расчета масштаба и смещения для центрирования карты
        function calculateScaleAndOffset() {
            var baseMapWidth = 1000
            var baseMapHeight = 700

            var availableWidth = width
            var availableHeight = height

            var scaleX = availableWidth / baseMapWidth
            var scaleY = availableHeight / baseMapHeight
            actualScale = Math.min(scaleX, scaleY)

            var scaledWidth = baseMapWidth * actualScale
            var scaledHeight = baseMapHeight * actualScale

            offsetX = (width - scaledWidth) / 2
            offsetY = (height - scaledHeight) / 2
        }

        // Функция рисования пути
        function drawPath(ctx, pathString, fillColor, lineWidth, strokeColor) {
            if (!pathString || pathString.length === 0) {
                return false
            }

            ctx.fillStyle = fillColor
            ctx.strokeStyle = strokeColor
            ctx.lineWidth = lineWidth

            ctx.beginPath()

            var tokens = pathString.replace(/([MLZ])/g, "|$1 ").split("|")
            var hasPoints = false

            for (var i = 0; i < tokens.length; i++) {
                var token = tokens[i].trim()
                if (!token) continue

                var parts = token.split(/\s+/)
                var cmd = parts[0]

                if (cmd === 'M' && parts.length >= 3) {
                    var mx = parseFloat(parts[1])
                    var my = parseFloat(parts[2])
                    if (!isNaN(mx) && !isNaN(my)) {
                        ctx.moveTo(mx, my)
                        hasPoints = true
                    }
                } else if (cmd === 'L' && parts.length >= 3) {
                    var lx = parseFloat(parts[1])
                    var ly = parseFloat(parts[2])
                    if (!isNaN(lx) && !isNaN(ly)) {
                        ctx.lineTo(lx, ly)
                        hasPoints = true
                    }
                } else if (cmd === 'Z') {
                    ctx.closePath()
                }
            }

            if (hasPoints) {
                ctx.fill()
                if (lineWidth > 0) {
                    ctx.stroke()
                }
                return true
            }

            return false
        }

        // Функция отрисовки сообщения об ошибке
        function drawErrorMessage(ctx, message) {
            ctx.fillStyle = "#333333"
            ctx.font = "16px Arial"
            ctx.textAlign = "center"
            ctx.textBaseline = "middle"
            ctx.fillText(message, width / 2, height / 2)
        }

        // Обработка кликов и hover
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton

            property bool clickInProgress: false
            property var currentHoveredRegion: null

            onClicked: function(mouse) {
                if (clickInProgress) return
                clickInProgress = true

                console.log("Клик на координатах:", mouse.x, mouse.y)

                // ОПТИМИЗАЦИЯ: Используем C++ метод для поиска региона
                var clickedRegion = mapData.getRegionAtPoint(
                    mouse.x, mouse.y,
                    mapCanvas.actualScale,
                    mapCanvas.offsetX,
                    mapCanvas.offsetY
                )

                if (clickedRegion && clickedRegion.id) {
                    console.log("Клик по региону:", clickedRegion.name, clickedRegion.id)

                    // Устанавливаем выбранный регион
                    mapData.selectedRegion = clickedRegion.id

                    // Уведомляем C++ о клике (испускает сигнал regionClicked)
                    mapData.notifyRegionClicked(clickedRegion.id, clickedRegion.name)

                    // Испускаем QML сигнал для обратной совместимости
                    mapComponent.regionClicked(clickedRegion.id, clickedRegion.name)
                } else {
                    console.log("Клик мимо региона - очищаем выбор")
                    mapData.clearSelection()
                }

                clickInProgress = false
            }

            // Throttle для hover - проверяем не чаще чем раз в 16мс (60 FPS)
            Timer {
                id: hoverThrottle
                interval: 16
                repeat: false
                property real pendingX: 0
                property real pendingY: 0
                property bool hasPending: false

                onTriggered: {
                    if (!hasPending) return
                    hasPending = false

                    // ОПТИМИЗАЦИЯ: Используем C++ метод для поиска региона
                    var hoveredRegion = mapData.getRegionAtPoint(
                        pendingX, pendingY,
                        mapCanvas.actualScale,
                        mapCanvas.offsetX,
                        mapCanvas.offsetY
                    )

                    if (hoveredRegion && hoveredRegion.id) {
                        parent.cursorShape = Qt.PointingHandCursor

                        if (!parent.currentHoveredRegion || parent.currentHoveredRegion.id !== hoveredRegion.id) {
                            parent.currentHoveredRegion = hoveredRegion
                            mapComponent.regionHovered(hoveredRegion.id, hoveredRegion.name)
                        }
                    } else {
                        parent.cursorShape = Qt.ArrowCursor

                        if (parent.currentHoveredRegion) {
                            parent.currentHoveredRegion = null
                            mapComponent.regionExited()
                        }
                    }
                }
            }

            onPositionChanged: function(mouse) {
                // Просто сохраняем координаты и запускаем таймер если он не запущен
                hoverThrottle.pendingX = mouse.x
                hoverThrottle.pendingY = mouse.y
                hoverThrottle.hasPending = true

                if (!hoverThrottle.running) {
                    hoverThrottle.start()
                }
            }

            onExited: {
                cursorShape = Qt.ArrowCursor
                currentHoveredRegion = null
                hoverThrottle.hasPending = false
                hoverThrottle.stop()
                mapComponent.regionExited()
            }
        }

        // Подключение к сигналам MapData
        Connections {
            target: mapData

            onRegionsChanged: {
                console.log("Сигнал: regionsChanged")
                // Подготавливаем оптимизированную геометрию для быстрого поиска
                mapData.prepareRegionGeometry()
                mapCanvas.requestPaint()
            }

            onSelectedRegionChanged: {
                console.log("Сигнал: selectedRegionChanged")
                if (!mapCanvas.isPainting) {
                    mapCanvas.requestPaint()
                }
            }
        }

        Component.onCompleted: {
            console.log("Canvas инициализирован")
            console.log("Размеры canvas:", width, "x", height)
            if (mapData) {
                console.log("MapData доступен, регионов:", mapData.regions ? mapData.regions.length : 0)
                // Подготавливаем геометрию при инициализации
                mapData.prepareRegionGeometry()
            } else {
                console.warn("MapData не доступен при инициализации Canvas!")
            }
            requestPaint()
        }

        // Перерисовка при изменении размера с debounce
        onWidthChanged: {
            if (width > 0) {
                console.log("Canvas width изменился:", width)
                resizeTimer.restart()
            }
        }

        onHeightChanged: {
            if (height > 0) {
                console.log("Canvas height изменился:", height)
                resizeTimer.restart()
            }
        }
    }

    Component.onCompleted: {
        console.log("=== MapComponent инициализирован ===")
        console.log("Размеры компонента:", width, "x", height)
        if (typeof mapData !== 'undefined') {
            console.log("MapData доступен, регионов:", mapData.regions ? mapData.regions.length : 0)
        } else {
            console.error("MapData НЕ доступен! Проверьте setContextProperty в C++")
        }
    }
}
