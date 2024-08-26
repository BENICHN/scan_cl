cmake_minimum_required(VERSION 3.28)
project(scan)

set(CMAKE_CXX_STANDARD 23)
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

find_package(OpenCV
        REQUIRED)

add_executable(scan
        resources.qrc
        src/main.cpp
        src/widgets/ImageViewerWidget.cpp
        src/widgets/ImageViewerWidget.h
        src/utils.h
        src/imports/stdimports.h
        src/imports/qtimports.h
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
        src/ProcessFragment.cpp
        src/ProcessFragment.h
        src/ProcessFragment.ui
        src/app.h
        src/app.cpp
        src/components/PreviewerWidget.cpp
        src/components/PreviewerWidget.h
        src/components/PreviewerWidget.ui
        src/colors.h
        src/data/book.cpp
        src/works.cpp
        src/works.h
        src/imports/qcoroimports.h
        src/imports/jsonimports.h
        src/imports/jsonimports.cpp
        src/steps/Step.cpp
        src/steps/Step.h
        src/steps/CroppingStep.cpp
        src/steps/CroppingStep.h
        src/steps/MergingStep.cpp
        src/steps/MergingStep.h
        src/steps/CleaningStep.cpp
        src/steps/CleaningStep.h
        src/steps/FinalStep.cpp
        src/steps/FinalStep.h
        src/widgets/PreviewerSelector.cpp
        src/widgets/PreviewerSelector.h
        src/widgets/PreviewerSelector.ui
        src/widgets/SelectionRectTypes.h
        src/components/PreviewerSelectionTypes.h
        src/widgets/PreviewerSelectorTypes.h
        src/imports/opencvimports.h
        src/imports/opencvimports.cpp
        src/components/models/PagesDelegate.cpp
        src/components/models/PagesDelegate.h
        src/components/props/PropsEditor.cpp
        src/components/props/PropsEditor.h
        src/components/props/PropsEditor.ui
        src/data/StaticJsonModel.cpp
        src/data/StaticJsonModel.h
        src/data/PageSettingsModel.cpp
        src/data/PageSettingsModel.h
        src/components/props/PropsEditorTypes.h
        src/Scanner.cpp
        src/Scanner.h
        src/components/ScanWidget.cpp
        src/components/ScanWidget.h
        src/components/ScanWidget.ui
        src/MainWindow.cpp
        src/MainWindow.h
        src/MainWindow.ui
        src/data/ScanOptionsModel.cpp
        src/data/ScanOptionsModel.h
        src/data/StaticJsonDelegate.cpp
        src/data/StaticJsonDelegate.h
        src/data/StaticJsonEditor.cpp
        src/data/StaticJsonEditor.h
        src/data/StaticJsonEditor.ui
        src/widgets/ScanButtons.cpp
        src/widgets/ScanButtons.h
        src/widgets/ScanButtons.ui
)
target_link_libraries(scan
        Qt::Core
        Qt::Gui
        Qt::Widgets
        QCoro::Core
        nlohmann_json::nlohmann_json
        xxhash
        ${OpenCV_LIBS}
        sane
)