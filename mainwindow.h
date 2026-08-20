#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QColor>
#include "bkground_helper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// ─── Clock-hand dialog ────────────────────────────────────────────────────────
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QGraphicsRectItem>

struct DeviceConfig {
    QString id;
    QString label;
    int canvasW;
    int canvasH;
    int anchorX;
    int anchorY;
    int previewW;
    int previewH;
    int previewCornerRadius;
    QColor previewBorderColor;
    int previewBorderWidth;
    int previewBorderRectWidth;
    int previewBorderRectHeight;
    double previewScale;
};

class ClockHandDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClockHandDialog(const QString &handName, int defaultAnchorX = 0, int defaultAnchorY = 0, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(QString("Configure %1 hand").arg(handName));
        setMinimumWidth(340);

        auto *grid   = new QGridLayout;
        auto *browse = new QPushButton("Browse…");

        fileEdit    = new QLineEdit;
        centerXSpin = new QSpinBox; centerXSpin->setRange(0, 9999); centerXSpin->setValue(0);
        centerYSpin = new QSpinBox; centerYSpin->setRange(0, 9999); centerYSpin->setValue(0);
        anchorXSpin = new QSpinBox; anchorXSpin->setRange(0, 9999); anchorXSpin->setValue(defaultAnchorX);
        anchorYSpin = new QSpinBox; anchorYSpin->setRange(0, 9999); anchorYSpin->setValue(defaultAnchorY);

        grid->addWidget(new QLabel("Image file:"), 0, 0);
        grid->addWidget(fileEdit,                  0, 1);
        grid->addWidget(browse,                    0, 2);
        grid->addWidget(new QLabel("centerX:"),    1, 0);
        grid->addWidget(centerXSpin,               1, 1, 1, 2);
        grid->addWidget(new QLabel("centerY:"),    2, 0);
        grid->addWidget(centerYSpin,               2, 1, 1, 2);
        grid->addWidget(new QLabel("anchorX:"),    3, 0);
        grid->addWidget(anchorXSpin,               3, 1, 1, 2);
        grid->addWidget(new QLabel("anchorY:"),    4, 0);
        grid->addWidget(anchorYSpin,               4, 1, 1, 2);

        auto *ok     = new QPushButton("OK");
        auto *cancel = new QPushButton("Cancel");
        auto *btnRow = new QHBoxLayout;
        btnRow->addStretch();
        btnRow->addWidget(ok);
        btnRow->addWidget(cancel);

        auto *root = new QVBoxLayout(this);
        root->addLayout(grid);
        root->addLayout(btnRow);

        connect(browse, &QPushButton::clicked, this, [this]{
            QString f = QFileDialog::getOpenFileName(this, "Select hand image", "",
                                                     "Images (*.png *.bmp)");
            if (!f.isEmpty()) {
                fileEdit->setText(f);
                // Auto-fill center = image size / 2
                QImage img(f);
                if (!img.isNull()) {
                    centerXSpin->setValue(img.width()  / 2);
                    centerYSpin->setValue(img.height() / 2);
                }
            }
        });
        connect(ok,     &QPushButton::clicked, this, &QDialog::accept);
        connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    }

    QString fileName()  const { return QFileInfo(fileEdit->text()).fileName(); }
    QString filePath()  const { return fileEdit->text(); }
    int  centerX()      const { return centerXSpin->value(); }
    int  centerY()      const { return centerYSpin->value(); }
    int  anchorX()      const { return anchorXSpin->value(); }
    int  anchorY()      const { return anchorYSpin->value(); }

private:
    QLineEdit *fileEdit;
    QSpinBox  *centerXSpin, *centerYSpin, *anchorXSpin, *anchorYSpin;
};

// ─── New-Project dialog ───────────────────────────────────────────────────────
class NewProjectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewProjectDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("New Project");
        setMinimumWidth(300);

        nameEdit    = new QLineEdit;
        auto *form  = new QFormLayout;
        form->addRow("Please enter a name:", nameEdit);

        auto *createBtn = new QPushButton("Create");
        auto *cancel    = new QPushButton("Cancel");
        auto *row       = new QHBoxLayout;
        row->addStretch();
        row->addWidget(createBtn);
        row->addWidget(cancel);

        auto *root = new QVBoxLayout(this);
        root->addLayout(form);
        root->addLayout(row);

        connect(createBtn, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancel,    &QPushButton::clicked, this, &QDialog::reject);
    }

    QString projectName() const { return nameEdit->text().trimmed(); }

private:
    QLineEdit *nameEdit;
};

// ─── MainWindow ───────────────────────────────────────────────────────────────
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void populateWidgetComboAdd();
    void populateTypeCombo();
    void onWidgetComboAddChanged(int index);

    void on_actionAbout_triggered();

    void pushButtonEditBkgroundPic();
    void onUploadBkgroundImg();
    void on_pushButtonCreateIwfLz_clicked();
    void onNewProject();
    void onAddWidget();
    void onUpdateChanges();
    void applyDeviceConfig(const QString &deviceId);

    void onXSpinChanged(int v);
    void onYSpinChanged(int v);
    void onWSpinChanged(int v);
    void onHSpinChanged(int v);

    void onCurrentWidgetComboChanged(int index);

    void onOpenIwfJson();
    void onSaveIwfJson();
    void onSaveFontJson();
    void onSavePreview();

private:
    Ui::MainWindow  *ui;
    bkground_helper *widgetListWindow = nullptr;

    QString  projectDir;
    bool     projectOpen  = false;
    QString currentProjectPath;
    int      fileCounter  = 0;

    struct WidgetEntry {
        QString             widgetType;
        QString             typeValue;
        QJsonObject         json;
        QGraphicsPixmapItem *sceneItem = nullptr; // rendered scene item
        // image strips per character key, only for custom widgets
        QMap<QString, QImage> imageStrip;
        QString              fontFolder; // subfolder name inside projectDir
    };
    QList<WidgetEntry> widgetList;
    int currentWidgetIndex = -1;

    QGraphicsPixmapItem *hourHandItem      = nullptr;
    QGraphicsPixmapItem *minHandItem       = nullptr;
    QGraphicsPixmapItem *secHandItem       = nullptr;
    QGraphicsRectItem   *selectionHighlight = nullptr; // yellow border around selected widget
    QMap<QString,QImage> loadImageStrip(const QString &folder) const;

    // font.json item list (mirrors plainTextEdit_2 / saved font.json)
    QJsonArray fontJsonItems;

    // ── scrollArea field table ────────────────────────────────────────────────
    struct FieldEntry {
        QString key;
        QString cbName;
        QString leName;
    };
    static const QList<FieldEntry> &scrollFields();

    QMap<QString, DeviceConfig> deviceConfigs;
    DeviceConfig currentDevice;

    // helpers
    void setProjectControlsEnabled(bool enabled);
    void rebuildCurrentWidgetCombo();
    void selectWidget(int index);
    void initDeviceConfigs();
    void updateHighlight();
    void updateJsonDisplay();
    void updateFontJsonDisplay();
    void loadScrollAreaFromJson(const QJsonObject &obj);
    void clearScrollArea();
    QJsonObject scrollAreaToJson(const QJsonObject &base) const;
    QString     prettyJson(const QJsonObject &root) const;
    QJsonObject buildRootJson() const;
    void        renderWatchHands();

    // custom widget rendering
    bool        addCustomWidget(const QString &typeVal);
    bool        addAnimaWidget();
    bool        addRedpointWidget();
    bool        addIconWidget();
    void        renderCustomWidget(int index, bool preserveSize = false);
    QImage      renderCustomWidgetImage(const WidgetEntry &e) const;
    QSize       measureCustomWidget(const WidgetEntry &e) const;
};

#endif // MAINWINDOW_H