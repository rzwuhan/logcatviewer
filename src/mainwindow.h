#ifndef LOGCATVIEWERWINDOW_H
#define LOGCATVIEWERWINDOW_H

#include <QMainWindow>
#include <QtGui/QTextCursor>
#include <QtWidgets/QTextEdit>
#include <QtCore/QVariant>
#include <qtmaterialcheckbox.h>
#include <qtmaterialtoggle.h>
#include <qtmaterialiconbutton.h>
#include <qtmaterialautocomplete.h>
#include <qtmaterialflatbutton.h>
#include <qtmaterialraisedbutton.h>
#include <qtmaterialsnackbar.h>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>

#include "config.h"
#include "adbhelper.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public ADBListener {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    // ADBListener
    virtual void onLogUpdate() override;

    virtual void onDeviceUpdate() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void updateDevices(QString devices);

    void updateLogWidget();

    void selectDevice(QString device);

    void selectFilter(QString key);

    void filtrating(QString keyword);

    void highlightFinding();

    void findPrev();

    void findNext();

    void clear();

Q_SIGNALS:

    void signalUpdateLogWidget();

private:
    Ui::MainWindow *ui;
    /**
     * 当前查找结果在ui->logs->extraSelections()中的索引
     */
    int current_selection;
    /**
     * 自定义tag过滤列表
     */
    QSet<QString> current_filter;

    /**
     * 快速过滤关键词
     */
    QString filter_keyword;
    /**
     * 搜索关键词
     */
    QString finder_keyword;
    const QColor FINDER_HIGHLIGHT_COLOR = QColor(Qt::yellow);
    /**
     * 是否自动滚动
     */
    bool auto_scroll;

    ADBHelper *adb;
};


namespace Ui {

class MainWindow {
public:
    QtMaterialAutoComplete *mDeviceSelectionWidget;
    QtMaterialTextField *mFilterKeywordWidget;
    QtMaterialAutoComplete *mFilterSelectionWidget;

    QtMaterialFlatButton *mClearWidget;
    QtMaterialFlatButton *mAutoScrolWidget;
    QtMaterialTextField *mSearchKeywordWidget;
    QtMaterialFlatButton *mSearchPrevButton;
    QtMaterialFlatButton *mSearchNextButton;
    QLabel *mSearchResultWidget;

    QPlainTextEdit *mLogWidget;

    QtMaterialSnackbar *mSnackbar;

public:
    void setupUi(QMainWindow *mainWindow) {
        mainWindow->resize(800, 600);
        QWidget *centralWidget = new QWidget(mainWindow);

        // VBOX
        //    HBOX [设备选择-过滤--自定义过滤器]
        //    HBOX [自动滚动-搜索-上一个-下一个]
        //    QPlainTextEdit
        // -----------------------------------
        QVBoxLayout *layout1 = new QVBoxLayout(centralWidget);
        // -----------------------------------
        QHBoxLayout *layout21 = new QHBoxLayout();
        layout21->setSpacing(10);
        mDeviceSelectionWidget = new QtMaterialAutoComplete(centralWidget);
        mDeviceSelectionWidget->setCursorPosition(0);
        mDeviceSelectionWidget->setShowInputLine(false);
        layout21->addWidget(mDeviceSelectionWidget);

        mFilterKeywordWidget = new QtMaterialTextField(centralWidget);
        layout21->addWidget(mFilterKeywordWidget);

        layout21->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

        mFilterSelectionWidget = new QtMaterialAutoComplete(centralWidget);
        mFilterSelectionWidget->setFixedWidth(100);
        layout21->addWidget(mFilterSelectionWidget);
        layout1->addLayout(layout21);

        QHBoxLayout *layout22 = new QHBoxLayout();
        mClearWidget = makeIconButton(centralWidget, "content", "delete_sweep");
        layout22->addWidget(mClearWidget);

        mAutoScrolWidget = makeIconButton(centralWidget, "action", "get_app");
        mAutoScrolWidget->setCheckable(true);
        layout22->addWidget(mAutoScrolWidget);

        mSearchKeywordWidget = new QtMaterialTextField(centralWidget);
        layout22->addWidget(mSearchKeywordWidget);

        mSearchPrevButton = makeIconButton(centralWidget, "navigation", "arrow_upward");
        layout22->addWidget(mSearchPrevButton);

        mSearchNextButton = makeIconButton(centralWidget, "navigation", "arrow_downward");
        layout22->addWidget(mSearchNextButton);

        mSearchResultWidget = new QLabel(centralWidget);
        mSearchResultWidget->setFixedWidth(100);
        layout22->addWidget(mSearchResultWidget);

        layout22->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

        layout1->addLayout(layout22);
        // ----------------------------------
        mLogWidget = new QPlainTextEdit(centralWidget);
        mLogWidget->setReadOnly(true);
        mLogWidget->setLineWrapMode(QPlainTextEdit::NoWrap);
        mLogWidget->setFont(QFont("Ubuntu Mono", 14));
        layout1->addWidget(mLogWidget);
        // ----------------------------------

        mSnackbar = new QtMaterialSnackbar(centralWidget);
        mainWindow->setCentralWidget(centralWidget);
    }

private:
    QtMaterialFlatButton *makeIconButton(QWidget *parent, QString category, QString icon) {
        QtMaterialFlatButton *button = new QtMaterialFlatButton(parent);
        button->setRole(Material::Primary);
        button->setFixedSize(32, 32);
        button->setIcon(QtMaterialTheme::icon(category, icon));
        button->setIconSize(QSize(24, 24));
        button->setIconPlacement(Material::CenterIcon);
        button->setRippleStyle(Material::NoRipple);
        button->setOverlayStyle(Material::TintedOverlay);
        return button;
    }
};

}

#endif // LOGCATVIEWERWINDOW_H
