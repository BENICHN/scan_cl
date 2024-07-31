cmake_minimum_required(VERSION 3.28)
project(scan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

find_package(QCoro6 COMPONENTS
        Core
        REQUIRED)

find_package(nlohmann_json
        REQUIRED)

add_executable(scan
        resources.qrc
        src/main.cpp
        src/widgets/ImageViewerWidget.cpp
        src/widgets/ImageViewerWidget.h
        src/utils.h
        src/stdimports.h
        src/qtimports.h
        src/utils.cpp
        src/mouse/MouseAction.cpp
        src/mouse/MouseAction.h
        src/mouse/MouseHandler.cpp
        src/mouse/MouseHandler.h
        src/widgets/SelectionRectWidget.cpp
        src/widgets/SelectionRectWidget.h
        src/widgets/TimelineWidget.cpp
        src/widgets/TimelineWidget.h
        src/widgets/TimelineWidget.ui
        src/data/book.h
        src/components/PageNavWidget.cpp
        src/components/PageNavWidget.h
        src/components/PageNavWidget.ui
        src/components/models/PagesModel.cpp
        src/components/models/PagesModel.h
        src/MainWindow.cpp
        src/MainWindow.h
        src/MainWindow.ui
        src/app.h
        src/app.cpp
        src/components/PreviewerWidget.cpp
        src/components/PreviewerWidget.h
        src/components/PreviewerWidget.ui
        src/colors.h
        src/data/book.cpp
        src/works.cpp
        src/works.h
        src/qcoroimports.h
        src/jsonimports.h
        src/jsonimports.cpp
)
target_link_libraries(scan
        Qt::Core
        Qt::Gui
        Qt::Widgets
        QCoro::Core
        nlohmann_json::nlohmann_json
)