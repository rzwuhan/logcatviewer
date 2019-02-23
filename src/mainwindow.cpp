#include <qdebug.h>
#include "mainwindow.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        current_selection(-1),
        filter_keyword(),
        finder_keyword(),
        auto_scroll(true),
        adb(new ADBHelper) {

    ui->setupUi(this);

    connect(ui->mClearWidget, &QPushButton::clicked, this, &MainWindow::clear);

    ui->mAutoScrolWidget->setChecked(auto_scroll);
    connect(ui->mAutoScrolWidget, &QAbstractButton::toggled, this, [this](int checked) {
        this->auto_scroll = checked;
    });

    ui->mDeviceSelectionWidget->setDataSource(adb->device_list);
    connect(ui->mDeviceSelectionWidget, &QtMaterialAutoComplete::itemSelected, this, [this](QString serial) {
        int index = adb->findDevice(serial);
        QString &status = adb->device_status[index];
        if (status == "offline" || status == "disconnect") {
            ui->mSnackbar->addMessage("[" + serial + "] 已断开");
        } else {
            adb->logcat(serial);
        }
    });

    if (!Config::GetInstance().hasFilter()) {
        ui->mFilterSelectionWidget->setDataSource(Config::GetInstance().getFilterKeys());
        connect(ui->mFilterSelectionWidget, &QtMaterialAutoComplete::itemSelected, this, &MainWindow::selectFilter);
    } else {
        ui->mFilterSelectionWidget->setVisible(false);
    }

    connect(ui->mSearchKeywordWidget, &QLineEdit::textChanged, this, &MainWindow::highlightFinding);
    connect(ui->mSearchPrevButton, &QPushButton::clicked, this, &MainWindow::findPrev);
    connect(ui->mSearchNextButton, &QPushButton::clicked, this, &MainWindow::findNext);
    connect(ui->mFilterKeywordWidget, &QLineEdit::textChanged, this, &MainWindow::filtrating);

    adb->setListener(this);

    connect(this, &MainWindow::signalUpdateLogWidget, this, &MainWindow::updateLogWidget);
}

MainWindow::~MainWindow() {
    delete ui;
    ui = nullptr;
    delete adb;
    adb = nullptr;
}

void MainWindow::updateLogWidget() {
    std::lock_guard<std::mutex> lck(adb->logs_lock);
    QTextCursor tmp(ui->mLogWidget->document());
    QTextBlockFormat bf = tmp.blockFormat();
    tmp.beginEditBlock();
    for (int limit = 0; adb->logs_current_index < adb->logs.size() && limit < 500; ++adb->logs_current_index, ++limit) {
        LogItem item = adb->logs[adb->logs_current_index];

        if (!filter_keyword.isEmpty() && !item.log.contains(filter_keyword, Qt::CaseInsensitive))
            continue;

        if (!current_filter.empty() && current_filter.contains(item.tag))
            continue;

        tmp.movePosition(QTextCursor::End);
        QTextCharFormat tf = ui->mLogWidget->currentCharFormat();
        tf.setForeground(item.Color());
        tmp.insertBlock(bf, tf);
        tmp.insertText(item.log.replace("\\", "\\\\"));
    }
    tmp.endEditBlock();

    // auto scroll
    if (auto_scroll) {
        QScrollBar *scrollbar = ui->mLogWidget->verticalScrollBar();
        if (scrollbar)
            scrollbar->setSliderPosition(scrollbar->maximum());
    }

    highlightFinding();
}

void MainWindow::selectFilter(QString key) {
    qDebug() << "select filter:" << key;
    current_filter = Config::GetInstance().getFilter(key);

    std::unique_lock<std::mutex> lck(adb->logs_lock);
    adb->logs_current_index = 0;
    lck.unlock();

    ui->mLogWidget->clear();
    updateLogWidget();
}

void MainWindow::filtrating(QString keyword) {
    if (keyword.size() > 0 && keyword.size() < 3) return;

    std::unique_lock<std::mutex> lck(adb->logs_lock);
    filter_keyword = keyword;
    adb->logs_current_index = 0;
    lck.unlock();

    ui->mLogWidget->clear();
    updateLogWidget();
}

void MainWindow::highlightFinding() {
    QString keyword = ui->mSearchKeywordWidget->text();
    if (keyword.size() > 0 && keyword.size() < 3) {
        ui->mSearchResultWidget->setText("");
        return;
    }
    QTextDocument *document = ui->mLogWidget->document();

    QList<QTextEdit::ExtraSelection> es;
    QTextCursor cursor;
    if (keyword != finder_keyword) {
        // 如果搜索关键词发生变化，则清空搜索结果，重新搜索
        finder_keyword = keyword;
        cursor = document->find(finder_keyword, QTextCursor(document));
        current_selection = -1;
    } else {
        // 如果搜索关键词没有变化，则更新搜索结果
        es = ui->mLogWidget->extraSelections();
        if (es.empty()) cursor = document->find(finder_keyword, QTextCursor(document));
        else cursor = es.last().cursor;
    }
//    qDebug() << "finding keyword : [" << mFinderKeyword << "] from : [" << cursor.position() << "]";
    for (; (!cursor.isNull() && !cursor.atEnd()); cursor = document->find(finder_keyword, cursor)) {
        QTextEdit::ExtraSelection selection;
        selection.cursor = cursor;
        selection.format.setBackground(FINDER_HIGHLIGHT_COLOR);
        es.append(selection);
    }
    ui->mLogWidget->setExtraSelections(es);
    if (!es.empty())
        ui->mSearchResultWidget->setText(QString::number(current_selection) + "/" + QString::number(es.size()));
}

void MainWindow::findPrev() {
    QList<QTextEdit::ExtraSelection> es = ui->mLogWidget->extraSelections();
    if (es.empty()) return;
    auto_scroll = false;
    current_selection = (current_selection - 1 + es.size()) % es.size();
    ui->mLogWidget->setTextCursor(es[current_selection].cursor);
    ui->mSearchResultWidget->setText(QString::number(current_selection) + "/" + QString::number(es.size()));
}

void MainWindow::findNext() {
    QList<QTextEdit::ExtraSelection> es = ui->mLogWidget->extraSelections();
    if (es.empty()) return;
    auto_scroll = false;
    current_selection = (current_selection + 1) % es.size();
    ui->mLogWidget->setTextCursor(es[current_selection].cursor);
    ui->mSearchResultWidget->setText(QString::number(current_selection) + "/" + QString::number(es.size()));
}

void MainWindow::clear() {
    std::lock_guard<std::mutex> lck(adb->logs_lock);
    adb->logs.clear();
    adb->logs_current_index = 0;
    ui->mLogWidget->clear();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
//    qDebug() << __FUNCTION__ << ": 0x" << hex << event->key();
    switch (event->key()) {
        case Qt::Key_F:
            // 搜索
            if (event->modifiers() & Qt::ControlModifier) {
                ui->mSearchKeywordWidget->selectAll();
                ui->mSearchKeywordWidget->setFocus();
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (ui->mSearchKeywordWidget->text().size() > 2) {
                // 上一个/下一个
                if (event->modifiers() & Qt::ShiftModifier) {
                    findPrev();
                } else {
                    findNext();
                }
            } else {
                // 切换自动滚动
                ui->mAutoScrolWidget->setChecked(!auto_scroll);
            }
            break;
        case Qt::Key_S:
            if (event->modifiers() & Qt::ControlModifier) {
                ui->mAutoScrolWidget->setChecked(!auto_scroll);
            }
            break;
        case Qt::Key_Escape:
            if (ui->mSearchKeywordWidget->text().size() > 2) {
                ui->mSearchKeywordWidget->clear();
                auto_scroll = true;
            }
            break;
    }
    QWidget::keyPressEvent(event);
}



void MainWindow::onLogUpdate() {
    emit signalUpdateLogWidget();
}

void MainWindow::onDeviceUpdate() {
    ui->mDeviceSelectionWidget->setDataSource(adb->device_list);
}