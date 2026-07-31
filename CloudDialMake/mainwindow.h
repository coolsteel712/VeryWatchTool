#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QPointer>
#include <cstdint>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSelectFolderClicked();
    void onBuildIwfClicked();
    void appendLog(const QString &text);

private:
    Ui::MainWindow *ui;
    QString m_selectedFolderPath;
    QComboBox *m_comboBoxFormat;
    QLabel *m_labelFormat;

    void setupCustomUi();
    void scanAndDetectFolder(const QString &folderPath);
    uint8_t getSelectedFormatByte() const;
    bool validateFolderStructure(const QString &folderPath);
};

#endif // MAINWINDOW_H