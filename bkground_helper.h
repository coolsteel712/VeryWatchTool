#ifndef BKGROUND_HELPER_H
#define BKGROUND_HELPER_H

#include <QWidget>
#include <QGraphicsScene>  // Add this

namespace Ui {
class bkground_helper;
}

class bkground_helper : public QWidget
{
    Q_OBJECT

public:
    explicit bkground_helper(QWidget *parent = nullptr);
    ~bkground_helper();

private slots:
    void upload_image();

private:
    Ui::bkground_helper *ui;
    QGraphicsScene *scene;
};

#endif // BKGROUND_HELPER_H
