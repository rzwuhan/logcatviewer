cmake_minimum_required(VERSION 3.10)
project(logcatviewer)

set(CMAKE_PREFIX_PATH /opt/qt/5.9.6/gcc_64/lib/cmake)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(thirdparty)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

aux_source_directory(src LOGCAT_VIEWER)

include_directories(
        thirdparty/qt-material-widgets/components
)

add_executable(logcatviewer ${LOGCAT_VIEWER})
# Use the Widgets module from Qt 5
target_link_libraries(logcatviewer pthread)
target_link_libraries(logcatviewer qtmaterialwidgets)
target_link_libraries(logcatviewer Qt5::Core Qt5::Widgets Qt5::Gui)
