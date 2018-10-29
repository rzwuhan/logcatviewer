cmake_minimum_required(VERSION 3.10)
project(guitools)

set(CMAKE_PREFIX_PATH /opt/qt/5.9.6/gcc_64/lib/cmake)
set(CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

aux_source_directory(src       LOGCAT_VIEWER)

add_executable(logcatviewer ${LOGCAT_VIEWER})
# Use the Widgets module from Qt 5
target_link_libraries(logcatviewer Qt5::Widgets pthread)