#ifndef DIALOGIWFLZ_H
#define DIALOGIWFLZ_H

#include <QDialog>

namespace Ui {
class DialogIwfLz;
}

class DialogIwfLz : public QDialog
{
    Q_OBJECT

public:
    explicit DialogIwfLz(QWidget *parent = nullptr);
    ~DialogIwfLz();

private:
    Ui::DialogIwfLz *ui;
};

#endif // DIALOGIWFLZ_H
