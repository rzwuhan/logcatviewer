#include <qdebug.h>
#include "logcatviewerwindow.h"
#include "ui_logcatviewerwindow.h"
#include <thread>
#include <QtWidgets/QMessageBox>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtWidgets/QScrollBar>

LogcatViewerWindow::LogcatViewerWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::LogcatViewerWindow),
        mLogcatProcess(NULL),
        mCurrentIndex(0),
        mCurrentSelection(-1),
        mFilterKeyword(),
        mFinderKeyword(),
        mAutoScroll(false),
        mStatusLineCount(new QLabel),
        mStatusFindResult(new QLabel) {

    ui->setupUi(this);
    // 状态栏布局
    ui->status->addPermanentWidget(mStatusLineCount);
    ui->status->addPermanentWidget(mStatusFindResult);

    // 预设空过滤列表
    ui->filters->addItem("no filters");
    mFilters.append(QSet<QString>());
    // 加载配置文件
    /*
    {
      "filters": [
        {"name": "filter1", "value": ["taga","tagb"]},
        {"name": "filter2", "value": ["tag1","tag2","tag3"]}
      ]
    }
    **/
    QFile file("logcatviewer.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonParseError e;
        QJsonDocument json = QJsonDocument::fromJson(file.readAll(), &e);
        if (e.error == QJsonParseError::NoError && !json.isNull()) {
            QJsonObject root = json.object();
            if (root.contains("filters")) {
                QJsonArray filters = root.take("filters").toArray();
                for (QJsonValue filter : filters) {
                    QSet<QString> value;
                    for (QJsonValue item : filter.toObject().take("value").toArray()) {
                        value.insert(item.toString());
                    }
                    mFilters.append(value);
                    ui->filters->addItem(filter.toObject().take("name").toString());
                }
            }
        } else {
            qDebug() << " 解析配置文件失败:" << e.errorString();
        }
    }

    connect(this, &LogcatViewerWindow::SignalUpdateDevices, this, &LogcatViewerWindow::UpdateDevices);
    connect(this, &LogcatViewerWindow::SignalUpdateLogs, this, &LogcatViewerWindow::UpdateLog);
    connect(ui->devices, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LogcatViewerWindow::SelectDevice);
    connect(ui->filters, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LogcatViewerWindow::SelectFilter);
    connect(ui->filter, &QLineEdit::textChanged, this, &LogcatViewerWindow::Filtrating);
    connect(ui->find, &QLineEdit::textChanged, this, &LogcatViewerWindow::HighlightFinding);
    connect(ui->findNext, &QPushButton::clicked, this, &LogcatViewerWindow::FindNext);
    connect(ui->findPrev, &QPushButton::clicked, this, &LogcatViewerWindow::FindPrev);
    connect(ui->clear, &QPushButton::clicked, this, &LogcatViewerWindow::Clear);
    connect(ui->autoscroll, &QPushButton::clicked, this, [this]() {
        this->mAutoScroll = !this->mAutoScroll;
    });

    std::thread device_thread([this] {
        QProcess *process = new QProcess;
        QStringList arguments;
        arguments << "devices";
        process->start(mProgram, arguments);
        process->waitForFinished();
        QString result = process->readAllStandardOutput();
        emit SignalUpdateDevices(result);
        delete process;
    });
    device_thread.detach();
}

LogcatViewerWindow::~LogcatViewerWindow() {
    delete ui;
    delete mStatusLineCount;
    delete mStatusFindResult;
}

void LogcatViewerWindow::UpdateDevices(QString result) {
    QStringList devices = result.split('\n', QString::SkipEmptyParts);
    for (int i = 1; i < devices.size(); ++i) {
        ui->devices->addItem(devices[i].split('\t', QString::SkipEmptyParts)[0]);
    }
}

void LogcatViewerWindow::UpdateLog() {
    mLocker.lock();
    LogItem item;
    QSet<QString> filters = mFilters[ui->filters->currentIndex()];
    QTextCursor tmp(ui->logs->document());
    QTextBlockFormat bf = tmp.blockFormat();
    tmp.beginEditBlock();
    for (int limit = 0; mCurrentIndex < mLogs.size() && limit < 500; ++mCurrentIndex, ++limit) {
        item = mLogs[mCurrentIndex];

        if (!mFilterKeyword.isEmpty() && !item.log.contains(mFilterKeyword, Qt::CaseInsensitive))
            continue;

        if (!filters.empty() && filters.contains(item.tag))
            continue;

        tmp.movePosition(QTextCursor::End);
        QTextCharFormat tf = ui->logs->currentCharFormat();
        tf.setForeground(item.Color());
        tmp.insertBlock(bf, tf);
        tmp.insertText(item.log.replace("\\", "\\\\"));
    }
    tmp.endEditBlock();

    // auto scroll
    if (mAutoScroll) {
        QScrollBar *scrollbar = ui->logs->verticalScrollBar();
        if (scrollbar)
            scrollbar->setSliderPosition(scrollbar->maximum());
    }

    HighlightFinding();

    mLocker.unlock();
    mStatusLineCount->setText(QString::number(mLogs.size()));
}

void LogcatViewerWindow::SelectDevice(int index) {
    // 初始化
    if (mLogcatProcess != NULL) {
        mLogcatProcess->terminate();
        mLogcatProcess->waitForFinished();
        delete mLogcatProcess;
        mLogcatProcess = NULL;
    }

    // 开启新线程，读取log
    std::thread thread([this] {
        QString device = ui->devices->currentText();
        mLogcatProcess = new QProcess;
        QStringList arguments;
        arguments << "-s" << device << "shell" << "logcat" << "-v" << "threadtime";
        qDebug() << arguments;
        mLogcatProcess->start(mProgram, arguments);
        if (mLogcatProcess->waitForStarted()) {
            char buffer[1024];
            while (mLogcatProcess->waitForReadyRead()) {
                mLocker.lock();
                while (true) {
                    if (mLogcatProcess->readLine(buffer, 1024) <= 0) break;
                    QString line = QString(buffer).trimmed();
                    if (!line.isEmpty()) {
                        mLogs.append(line);
                    }
                }
                mLocker.unlock();
                emit SignalUpdateLogs();
            }
            qDebug() << "wait for ready read exit : " << mLogcatProcess->errorString();
        } else {
            qDebug() << "wait for started failed : " << mLogcatProcess->errorString();
        }
        if (!mLogcatProcess->waitForFinished())
            qDebug() << "wait for finished failed : " << mLogcatProcess->errorString();
        mLogcatProcess->close();
        qDebug() << "logcat thread exit!";
    });
    thread.detach();
}

void LogcatViewerWindow::SelectFilter(int index) {
    qDebug() << mFilters[index];
    mLocker.lock();
    ui->logs->clear();
    mCurrentIndex = 0;
    mLocker.unlock();
}

void LogcatViewerWindow::Filtrating() {
    QString keyword = ui->filter->text();
    if (keyword.size() == 1 || keyword.size() == 2) return;
    mLocker.lock();
    ui->logs->clear();
    mFilterKeyword = keyword;
    mCurrentIndex = 0;
    mLocker.unlock();
}

void LogcatViewerWindow::HighlightFinding() {
    QString keyword = ui->find->text();
    if (keyword.size() == 1 || keyword.size() == 2) return;
    QTextDocument *document = ui->logs->document();

    QList<QTextEdit::ExtraSelection> es;
    QTextCursor cursor;
    if (keyword != mFinderKeyword) {
        // 如果搜索关键词发生变化，则清空搜索结果，重新搜索
        mFinderKeyword = keyword;
        cursor = document->find(mFinderKeyword, QTextCursor(document));
        mCurrentSelection = -1;
    } else {
        // 如果搜索关键词没有变化，则更新搜索结果
        es = ui->logs->extraSelections();
        if (es.empty()) cursor = document->find(mFinderKeyword, QTextCursor(document));
        else cursor = es.last().cursor;
    }
//    qDebug() << "finding keyword : [" << mFinderKeyword << "] from : [" << cursor.position() << "]";
    for (; (!cursor.isNull() && !cursor.atEnd()); cursor = document->find(mFinderKeyword, cursor)) {
        QTextEdit::ExtraSelection selection;
        selection.cursor = cursor;
        selection.format.setBackground(FINDER_HIGHLIGHT_COLOR);
        es.append(selection);
    }
    ui->logs->setExtraSelections(es);
    mStatusFindResult->setText(QString::number(mCurrentSelection) + "/" + QString::number(es.size()));
}

void LogcatViewerWindow::FindPrev() {
    QList<QTextEdit::ExtraSelection> es = ui->logs->extraSelections();
    if (es.empty()) return;
    mAutoScroll = false;
    mCurrentSelection = (mCurrentSelection - 1 + es.size()) % es.size();
    ui->logs->setTextCursor(es[mCurrentSelection].cursor);
    mStatusFindResult->setText(QString::number(mCurrentSelection) + "/" + QString::number(es.size()));
}

void LogcatViewerWindow::FindNext() {
    QList<QTextEdit::ExtraSelection> es = ui->logs->extraSelections();
    if (es.empty()) return;
    mAutoScroll = false;
    mCurrentSelection = (mCurrentSelection + 1) % es.size();
    ui->logs->setTextCursor(es[mCurrentSelection].cursor);
    mStatusFindResult->setText(QString::number(mCurrentSelection) + "/" + QString::number(es.size()));
}

void LogcatViewerWindow::Clear() {
    mLocker.lock();
    mLogs.clear();
    ui->logs->clear();
    mCurrentIndex = 0;
    mLocker.unlock();
    emit UpdateLog();
}

void LogcatViewerWindow::keyPressEvent(QKeyEvent *event) {
//    qDebug() << __FUNCTION__ << ": 0x" << hex << event->key();
    switch (event->key()) {
        case Qt::Key_F:
            // 搜索
            if (event->modifiers() & Qt::ControlModifier) {
                ui->find->selectAll();
                ui->find->setFocus();
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (ui->find->text().size() > 2) {
                // 上一个/下一个
                if (event->modifiers() & Qt::ShiftModifier) {
                    FindPrev();
                } else {
                    FindNext();
                }
            } else {
                // 切换自动滚动
                mAutoScroll = !mAutoScroll;
            }
            break;
        case Qt::Key_Escape:
            if (ui->find->text().size() > 2) {
                ui->find->clear();
                mAutoScroll = true;
            }
            break;
    }
    QWidget::keyPressEvent(event);
}
