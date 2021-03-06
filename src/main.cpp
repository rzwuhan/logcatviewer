#include "mainwindow.h"
#include <QApplication>
#include <qscreen.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(resources);

    QScreen *screen = QApplication::primaryScreen();
    MainWindow mainWindow;
    int x = (screen->size().width() - mainWindow.width()) / 2;
    int y = (screen->size().height() - mainWindow.height()) / 2;
    mainWindow.move(x, y);
    mainWindow.show();

    return app.exec();
}
