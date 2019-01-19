#ifndef LOGCATVIEWER_CONFIG_H
#define LOGCATVIEWER_CONFIG_H

#include <QList>
#include <QString>
#include <QSet>

class Config {
public:
    QStringList mFilterKeys;
    QVector<QSet<QString>> mFilterValues;

public:
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
