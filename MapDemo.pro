QT += core qml quick widgets

CONFIG += c++11

TARGET = MapDemo
TEMPLATE = app

# Исходные файлы
SOURCES += \
    main.cpp \
    mapdata.cpp

HEADERS += \
    mapdata.h

# QML файлы
RESOURCES += \
    resources.qrc

# QML модули
QML_IMPORT_PATH = .

# Папки для сборки
DESTDIR = bin
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui