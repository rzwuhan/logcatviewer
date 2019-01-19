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
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <thirdparty/qt-material-widgets/components/qtmaterialsnackbar.h>

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

    /**
     *
     * 加载配置文件
     *
     * {
     *     "filters":
     *     [
     *         {"name": "filter1", "value": ["taga","tagb"]},
     *         {"name": "filter2", "value": ["tag1","tag2","tag3"]}
     *     ]
     * }
     *
     **/
    void loadConfig();

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
    Config config;
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

    QtMaterialFlatButton *mAutoScrolWidget;
    QtMaterialTextField *mSearchKeywordWidget;
    QtMaterialRaisedButton *mSearchPrevButton;
    QtMaterialRaisedButton *mSearchNextButton;
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
        mAutoScrolWidget = new QtMaterialFlatButton(centralWidget);
        mAutoScrolWidget->setRole(Material::Role::Primary);
        mAutoScrolWidget->setFixedSize(28, 28);
        mAutoScrolWidget->setIcon(QtMaterialTheme::icon("action", "get_app"));
        mAutoScrolWidget->setIconSize(QSize(25, 25));
        mAutoScrolWidget->setFocusPolicy(Qt::NoFocus);
        mAutoScrolWidget->setRippleStyle(Material::RippleStyle::NoRipple);
        mAutoScrolWidget->setOverlayStyle(Material::OverlayStyle::TintedOverlay);
        mAutoScrolWidget->setCheckable(true);
        layout22->addWidget(mAutoScrolWidget);

        mSearchKeywordWidget = new QtMaterialTextField(centralWidget);
        layout22->addWidget(mSearchKeywordWidget);

        mSearchPrevButton = new QtMaterialRaisedButton(centralWidget);
        mSearchPrevButton->setRole(Material::Role::Primary);
        mSearchPrevButton->setFixedSize(28, 28);
        mSearchPrevButton->setIcon(QtMaterialTheme::icon("navigation", "arrow_upward"));
        mSearchPrevButton->setIconSize(QSize(25, 25));
        layout22->addWidget(mSearchPrevButton);

        mSearchNextButton = new QtMaterialRaisedButton(centralWidget);
        mSearchNextButton->setRole(Material::Role::Primary);
        mSearchNextButton->setFixedSize(28, 28);
        mSearchNextButton->setIcon(QtMaterialTheme::icon("navigation", "arrow_downward"));
        mSearchNextButton->setIconSize(QSize(25, 25));
        layout22->addWidget(mSearchNextButton);

        mSearchResultWidget = new QLabel(centralWidget);
        mSearchResultWidget->setFixedWidth(100);
        layout22->addWidget(mSearchResultWidget);

        layout22->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

        layout1->addLayout(layout22);
        // ----------------------------------
        mLogWidget = new QPlainTextEdit(centralWidget);
        mLogWidget->setReadOnly(true);
        mLogWidget->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
        mLogWidget->setFont(QFont("Ubuntu Mono", 14));
        layout1->addWidget(mLogWidget);
        // ----------------------------------

        mSnackbar = new QtMaterialSnackbar(centralWidget);
        mainWindow->setCentralWidget(centralWidget);
    }
};

}

#endif // LOGCATVIEWERWINDOW_H
