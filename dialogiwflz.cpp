#include "dialogiwflz.h"
#include "ui_dialogiwflz.h"

DialogIwfLz::DialogIwfLz(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogIwfLz)
{
    ui->setupUi(this);
}

DialogIwfLz::~DialogIwfLz()
{
    delete ui;
}
