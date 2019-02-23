#ifndef LOGCATVIEWER_CONFIG_H
#define LOGCATVIEWER_CONFIG_H

#include <QList>
#include <QString>
#include <QSet>
#include <QDebug>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

class Config {

private:
    Config() {
        loadConfig();
    }

    Config(const Config &);

    Config &operator=(const Config &);

public:
    static Config &GetInstance() {
        static Config instance;
        return instance;
    }

public:
    QStringList mFilterKeys;
    QVector<QSet<QString>> mFilterValues;
    QString mADB;

public:
    /**
     *
     * 加载配置文件
     *
     * {
     *     "adb":"/opt/androidsdk/platform-tools/adb",
     *     "filters":
     *     [
     *         {"name": "filter1", "value": ["taga","tagb"]},
     *         {"name": "filter2", "value": ["tag1","tag2","tag3"]}
     *     ]
     * }
     *
     **/
    void loadConfig() {
        QFile file("~/.logcatviewer.json");
        if (file.open(QIODevice::ReadOnly)) {
            QJsonParseError e;
            QJsonDocument json = QJsonDocument::fromJson(file.readAll(), &e);
            if (e.error == QJsonParseError::NoError && !json.isNull()) {
                QJsonObject root = json.object();
                if (root.contains("adb")) {
                    mADB = root.take("adb").toString();
                }

                if (root.contains("filters")) {
                    QJsonArray filters = root.take("filters").toArray();
                    for (QJsonValue filter : filters) {
                        QSet<QString> value;
                        for (QJsonValue item : filter.toObject().take("value").toArray()) {
                            value.insert(item.toString());
                        }
                        addFilter(filter.toObject().take("name").toString(), value);
                    }
                }
            } else {
                qDebug() << " 解析配置文件失败:" << e.errorString();
            }
        }

        if (mADB.isEmpty())
            mADB = "/opt/androidsdk/platform-tools/adb";
    }

    void addFilter(QString key, QSet<QString> &values) {
        mFilterKeys.append(key);
        mFilterValues.append(values);
    }

    const QStringList &getFilterKeys() const {
        return mFilterKeys;
    }

    bool hasFilter() const {
        return mFilterKeys.empty();
    }

    const QSet<QString> &getFilter(QString &key) const {
        int index = 0;
        for (; index < mFilterKeys.size(); ++index)
            if (mFilterKeys[index] == key)
                break;
        return mFilterValues[index];
    }
};


#endif //LOGCATVIEWER_CONFIG_H
