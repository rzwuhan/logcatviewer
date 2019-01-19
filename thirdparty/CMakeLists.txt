set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON) # 自动处理slot和signal
set(CMAKE_AUTORCC ON) # 自动处理rc资源文件
set(CMAKE_AUTOUIC ON) # 自动处理ui界面文件

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

include_directories(
        qt-material-widgets/components
        qt-material-widgets/components/lib
        qt-material-widgets/components/layouts
)

# -----------------------------------------------------------------------------------------
# qt material widgets library
aux_source_directory(qt-material-widgets/components QT_MATERIAL_WIDGETS)
aux_source_directory(qt-material-widgets/components/lib QT_MATERIAL_WIDGETS)
aux_source_directory(qt-material-widgets/components/layouts QT_MATERIAL_WIDGETS)

add_library(qtmaterialwidgets STATIC
        ${QT_MATERIAL_WIDGETS}
        qt-material-widgets/components/resources.qrc)
target_link_libraries(qtmaterialwidgets Qt5::Core Qt5::Widgets Qt5::Gui)

# -----------------------------------------------------------------------------------------
# example
aux_source_directory(qt-material-widgets/examples QT_MATERIAL_WIDGETS_EXAMPLE)
add_executable(qtmaterialwidgets_example
        ${QT_MATERIAL_WIDGETS_EXAMPLE}
        qt-material-widgets/examples/examples.qrc)
target_link_libraries(qtmaterialwidgets_example qtmaterialwidgets)
target_link_libraries(qtmaterialwidgets_example Qt5::Core Qt5::Widgets Qt5::Gui)
