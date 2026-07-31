#ifndef TOOL_UTILS_H
#define TOOL_UTILS_H

#include <QObject>

class tool_utils : public QObject
{
    Q_OBJECT
public:
    explicit tool_utils(QObject *parent = nullptr);

signals:
};

#endif // TOOL_UTILS_H
