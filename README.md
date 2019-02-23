# logcatviewer

logcat viewer for android with qt5

# thirdparty

[qt-material-widgets](https://github.com/laserpants/qt-material-widgets)

# deploy

```bash
linuxdeployqt logcatviewer                      \
    -qmake=/opt/qt/5.12.0/gcc_64/bin/qmake       \
    -appimage -no-translations                  \
    -extra-plugins=iconengines/libqsvgicon.so
```
