#include "mainwindow.h"
#include "dialogiwflz.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPixmap>
#include <QTransform>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QCheckBox>
#include <QLineEdit>
#include <QInputDialog>
#include <QJsonParseError>
#include <QDebug>
#include <cmath>

// ─── Static preview time (mirrors Python: "10:08" / second "36") ─────────────
static const int PREVIEW_HOUR = 10;
static const int PREVIEW_MIN  = 8;
static const int PREVIEW_SEC  = 36;

// ─── scrollArea field table ───────────────────────────────────────────────────
//
// Complete row-by-row mapping extracted from mainwindow.ui.
// Row 0: QLabel "Widget" + lineEdit  (display-only — widget/type shown separately)
// Row 1: QLabel "Type"   + lineEdit  (display-only)
// Rows 2-46: QCheckBox (col 0, key enabled toggle) + QLineEdit (col 1, value)
//
// Layout (sorted by row):
//   Row  2 : fgcolor       checkBox_2   lineEdit_4
//   Row  3 : bgcolor       checkBox_3   lineEdit_5
//   Row  4 : fgrender      checkBox_4   lineEdit_6
//   Row  5 : bgrender      checkBox_5   lineEdit_7
//   Row  6 : bg            checkBox_6   lineEdit_8
//   Row  7 : align         checkBox_7   lineEdit_9
//   Row  8 : style         checkBox_8   lineEdit_48
//   Row  9 : follow        checkBox_9   lineEdit_49
//   Row 10 : target        checkBox_10  lineEdit_50
//   Row 11 : font          checkBox_11  lineEdit_10
//   Row 12 : fontnum       checkBox_12  lineEdit_11
//   Row 13 : numwidth      checkBox_13  lineEdit_12
//   Row 14 : time          checkBox_14  lineEdit_13
//   Row 15 : turn          checkBox_15  lineEdit_14
//   Row 16 : animatype     checkBox_16  lineEdit_51
//   Row 17 : animaicon     checkBox_17  lineEdit_15
//   Row 18 : frame         checkBox_18  lineEdit_16
//   Row 19 : animabpp      checkBox_19  lineEdit_17
//   Row 20 : animaformat   checkBox_20  lineEdit_18
//   Row 21 : app           checkBox_21  lineEdit_19
//   Row 22 : panchorx      checkBox_22  lineEdit_20
//   Row 23 : panchory      checkBox_23  lineEdit_21
//   Row 24 : pcenterx      checkBox_24  lineEdit_23
//   Row 25 : pcentery      checkBox_25  lineEdit_22
//   Row 26 : startangle    checkBox_27  lineEdit_24
//   Row 27 : endangle      checkBox_26  lineEdit_25
//   Row 28 : content       checkBox_28  lineEdit_26
//   Row 29 : direction     checkBox_29  lineEdit_27
//   Row 30 : pointer       checkBox_30  lineEdit_28
//   Row 31 : ringedge      checkBox_33  lineEdit_31
//   Row 32 : second        checkBox_35  lineEdit_33
//   Row 33 : seccenterx    checkBox_36  lineEdit_34
//   Row 34 : seccentery    checkBox_37  lineEdit_35
//   Row 35 : secanchorx    checkBox_38  lineEdit_36
//   Row 36 : secanchory    checkBox_39  lineEdit_37
//   Row 37 : minute        checkBox_41  lineEdit_38
//   Row 38 : mincenterx    checkBox_40  lineEdit_39
//   Row 39 : mincentery    checkBox_43  lineEdit_42
//   Row 40 : minanchorx    checkBox_42  lineEdit_41
//   Row 41 : minanchory    checkBox_44  lineEdit_40
//   Row 42 : hour          checkBox_46  lineEdit_46
//   Row 43 : hourcenterx   checkBox_45  lineEdit_45
//   Row 44 : hourcentery   checkBox_49  lineEdit_43
//   Row 45 : houranchorx   checkBox_48  lineEdit_47
//   Row 46 : houranchory   checkBox_47  lineEdit_44

const QList<MainWindow::FieldEntry> &MainWindow::scrollFields()
{
    static const QList<FieldEntry> fields = {
                                             { "fgcolor",     "checkBox_2",  "lineEdit_4"  },
                                             { "bgcolor",     "checkBox_3",  "lineEdit_5"  },
                                             { "fgrender",    "checkBox_4",  "lineEdit_6"  },
                                             { "bgrender",    "checkBox_5",  "lineEdit_7"  },
                                             { "bg",          "checkBox_6",  "lineEdit_8"  },
                                             { "align",       "checkBox_7",  "lineEdit_9"  },
                                             { "style",       "checkBox_8",  "lineEdit_48" },
                                             { "follow",      "checkBox_9",  "lineEdit_49" },
                                             { "target",      "checkBox_10", "lineEdit_50" },
                                             { "font",        "checkBox_11", "lineEdit_10" },
                                             { "fontnum",     "checkBox_12", "lineEdit_11" },
                                             { "numwidth",    "checkBox_13", "lineEdit_12" },
                                             { "time",        "checkBox_14", "lineEdit_13" },
                                             { "turn",        "checkBox_15", "lineEdit_14" },
                                             { "animatype",   "checkBox_16", "lineEdit_51" },
                                             { "animaicon",   "checkBox_17", "lineEdit_15" },
                                             { "frame",       "checkBox_18", "lineEdit_16" },
                                             { "animabpp",    "checkBox_19", "lineEdit_17" },
                                             { "animaformat", "checkBox_20", "lineEdit_18" },
                                             { "app",         "checkBox_21", "lineEdit_19" },
                                             { "panchorx",    "checkBox_22", "lineEdit_20" },
                                             { "panchory",    "checkBox_23", "lineEdit_21" },
                                             { "pcenterx",    "checkBox_24", "lineEdit_23" },
                                             { "pcentery",    "checkBox_25", "lineEdit_22" },
                                             { "startangle",  "checkBox_27", "lineEdit_24" },
                                             { "endangle",    "checkBox_26", "lineEdit_25" },
                                             { "content",     "checkBox_28", "lineEdit_26" },
                                             { "direction",   "checkBox_29", "lineEdit_27" },
                                             { "pointer",     "checkBox_30", "lineEdit_28" },
                                             { "ringedge",    "checkBox_33", "lineEdit_31" },
                                             { "second",      "checkBox_35", "lineEdit_33" },
                                             { "seccenterx",  "checkBox_36", "lineEdit_34" },
                                             { "seccentery",  "checkBox_37", "lineEdit_35" },
                                             { "secanchorx",  "checkBox_38", "lineEdit_36" },
                                             { "secanchory",  "checkBox_39", "lineEdit_37" },
                                             { "minute",      "checkBox_41", "lineEdit_38" },
                                             { "mincenterx",  "checkBox_40", "lineEdit_39" },
                                             { "mincentery",  "checkBox_43", "lineEdit_42" },
                                             { "minanchorx",  "checkBox_42", "lineEdit_41" },
                                             { "minanchory",  "checkBox_44", "lineEdit_40" },
                                             { "hour",        "checkBox_46", "lineEdit_46" },
                                             { "hourcenterx", "checkBox_45", "lineEdit_45" },
                                             { "hourcentery", "checkBox_49", "lineEdit_43" },
                                             { "houranchorx", "checkBox_48", "lineEdit_47" },
                                             { "houranchory", "checkBox_47", "lineEdit_44" },
                                             };
    return fields;
}

// ─── Helpers to find scroll-area widgets by name ──────────────────────────────
static QCheckBox *findCB(QWidget *root, const QString &name)
{
    return root->findChild<QCheckBox *>(name);
}
static QLineEdit *findLE(QWidget *root, const QString &name)
{
    return root->findChild<QLineEdit *>(name);
}

// ─── Constructor ──────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initDeviceConfigs();

    // ── Connect model combo box and set default active device ──────────────────
    connect(ui->comboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::applyDeviceConfig);

    // Apply initial selected model (e.g. IDW20)
    applyDeviceConfig(ui->comboBox->currentText());

    connect(ui->comboBox, &QComboBox::currentTextChanged,
            this, &MainWindow::applyDeviceConfig);

    // ─── Configure View Properties for Automatic Centering ────────────────────
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setStyleSheet("background-color: black;");

    // Tell Qt to pin the viewport center to the scene's center point
    ui->graphicsView->setAlignment(Qt::AlignCenter);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    // Create scene and bind signal for resolution updates
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    // Whenever sceneRect changes (e.g. switching resolutions), re-center automatically
    connect(scene, &QGraphicsScene::sceneRectChanged, ui->graphicsView, [this](const QRectF &rect) {
        ui->graphicsView->centerOn(rect.center());
    });

    // ✨ KEY: Set the view to scale the scene by 2x
    // The view is 480x576 (which equals 320*1.5, 385*1.5)
    // To get exactly 2x, we need to scale by 2.0

    // Option A: If you want exactly 2x (640x770 would be needed for full view)
    // But your view is 480x576, so 1.5x is the max that fits
    // Let's calculate the actual scale that fits the view:
    // double scaleX = 480.0 / 320.0;  // = 1.5
    // double scaleY = 576.0 / 385.0;  // ≈ 1.496
    // double scale = qMin(scaleX, scaleY); // Use the smaller to fit both dimensions

    // Apply the scale to the view's transform
    // QTransform transform;
    // transform.scale(scale, scale);
    // ui->graphicsView->setTransform(transform);

    // "New widget" combos
    connect(ui->widgetComboAdd_2, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onWidgetComboAddChanged);
    populateWidgetComboAdd();

    // Buttons
    connect(ui->pushButtonEditBkgroundPic,  &QPushButton::clicked, this, &MainWindow::pushButtonEditBkgroundPic);
    connect(ui->pushButtonUploadBkground,   &QPushButton::clicked, this, &MainWindow::onUploadBkgroundImg);
    connect(ui->pushButtonCreateIwfLz_6,    &QPushButton::clicked, this, &MainWindow::onNewProject);
    connect(ui->pushButtonAjouterWidget_2,  &QPushButton::clicked, this, &MainWindow::onAddWidget);
    connect(ui->pushButtonAppliquer_2,      &QPushButton::clicked, this, &MainWindow::onUpdateChanges);
    connect(ui->pushButtonCreerPreview_2,   &QPushButton::clicked, this, &MainWindow::onSavePreview);

    // X/Y/W/H real-time
    connect(ui->xSpin_2, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onXSpinChanged);
    connect(ui->ySpin_2, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onYSpinChanged);
    connect(ui->wSpin_2, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onWSpinChanged);
    connect(ui->hSpin_2, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onHSpinChanged);

    // "Current widget" combo
    connect(ui->currentWidgetCombo_2, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCurrentWidgetComboChanged);

    // Menu actions
    connect(ui->actionOpen_iwf_json,  &QAction::triggered, this, &MainWindow::onOpenIwfJson);
    connect(ui->actionSave_iwf_json,  &QAction::triggered, this, &MainWindow::onSaveIwfJson);
    connect(ui->actionSave_font_json, &QAction::triggered, this, &MainWindow::onSaveFontJson);

    // Disable everything except New Project at startup
    setProjectControlsEnabled(false);
    clearScrollArea();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initDeviceConfigs()
{
    deviceConfigs["IDW13"] = {
        "IDW13", "IDW13", 240, 284, 120, 142, 174, 196, 31,
        QColor(37, 37, 37), 2, 168, 194, 0.97
    };

    deviceConfigs["IDW18"] = {
        "IDW18", "IDW18", 240, 240, 120, 120, 180, 180, 84,
        QColor(123, 123, 123), 2, 169, 169, 0.94
    };

    deviceConfigs["IDW20"] = {
        "IDW20", "IDW20", 320, 385, 160, 193, 272, 324, 67,
        QColor(128, 128, 128), 3, 268, 320, 0.95
    };
}


void MainWindow::applyDeviceConfig(const QString &modelName)
{
    if (!deviceConfigs.contains(modelName))
        return;

    currentDevice = deviceConfigs[modelName];

    // Read current editor state
    QJsonObject root = buildRootJson();

    // Force update model fields to match newly selected device
    root["description"] = currentDevice.id;
    root["deviceId"]    = currentDevice.id;

    // Output formatted JSON back to the editor via prettyJson
    ui->plainTextEdit->setPlainText(prettyJson(root));
}

// ─── Enable / disable controls ────────────────────────────────────────────────
void MainWindow::setProjectControlsEnabled(bool enabled)
{
    ui->widgetComboAdd_2->setEnabled(enabled);
    ui->typeComboAdd_2->setEnabled(enabled);
    ui->pushButtonAjouterWidget_2->setEnabled(enabled);
    ui->pushButtonCreateIwf->setEnabled(enabled);
    ui->pushButtonCreateIwfLz->setEnabled(enabled);

    ui->currentWidgetCombo_2->setEnabled(enabled);
    ui->currentTypeCombo_2->setEnabled(enabled);
    ui->pushButtonAppliquer_2->setEnabled(enabled);
    ui->pushButtonSupprimerWidget_2->setEnabled(enabled);
    ui->xSpin_2->setEnabled(enabled);
    ui->ySpin_2->setEnabled(enabled);
    ui->wSpin_2->setEnabled(enabled);
    ui->hSpin_2->setEnabled(enabled);

    ui->pushButtonUploadBkground->setEnabled(enabled);
    ui->pushButtonEditBkgroundPic->setEnabled(enabled);
    ui->pushButtonCreerPreview_2->setEnabled(enabled);
}

// ─── New Project ──────────────────────────────────────────────────────────────
void MainWindow::onNewProject()
{
    NewProjectDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    QString name = dlg.projectName();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Project name cannot be empty.");
        return;
    }

    QString baseDir = QFileDialog::getExistingDirectory(this,
                                                        "Select directory to place the project", QDir::homePath());
    if (baseDir.isEmpty()) return;

    projectDir = baseDir + "/" + name;
    QDir d;
    if (!d.mkpath(projectDir)) {
        QMessageBox::critical(this, "Error",
                              "Could not create project folder:\n" + projectDir);
        return;
    }

    widgetList.clear();
    fileCounter  = 0;
    projectOpen  = true;
    currentWidgetIndex = -1;

    // ✨ Clear all GraphicsView items
    QGraphicsScene *scene = ui->graphicsView->scene();
    if (scene) {
        scene->clear();
        // Reset hand item pointers since scene->clear() deletes them
        hourHandItem = nullptr;
        minHandItem = nullptr;
        secHandItem = nullptr;
        selectionHighlight = nullptr;
    }

    // ✨ Reset font.json items to empty array
    fontJsonItems = QJsonArray();
    updateFontJsonDisplay(); // This will show {"item":[]} in the font text edit

    QJsonObject root;
    root["version"]          = 1;
    root["clouddialversion"] = 3;
    root["preview"]          = "preview.png";
    root["name"]             = name;
    root["author"]           = "admin";
    root["description"]      = currentDevice.id.isEmpty() ? ui->comboBox->currentText() : currentDevice.id;
    root["deviceId"]         = currentDevice.id.isEmpty() ? ui->comboBox->currentText() : currentDevice.id;
    root["bluetooth"]        = false;
    root["disturb"]          = false;
    root["battery"]          = false;
    root["compress"]         = "LZ4";
    root["item"]             = QJsonArray();
    root["bkground"]         = "";

    ui->plainTextEdit->setPlainText(prettyJson(root));
    clearScrollArea();
    rebuildCurrentWidgetCombo();
    setProjectControlsEnabled(true);

    QMessageBox::information(this, "Project Created",
                             "Project \"" + name + "\" created at:\n" + projectDir);

    ui->comboBox->setEnabled(false); // Added to prevent issues
}

// ─── Combo helpers ────────────────────────────────────────────────────────────
void MainWindow::populateWidgetComboAdd()
{
    QStringList widgets = { "custom", "watch", "ring", "progressbar" };
    ui->widgetComboAdd_2->clear();
    ui->widgetComboAdd_2->addItems(widgets);
}

void MainWindow::onWidgetComboAddChanged(int index)
{
    Q_UNUSED(index);
    populateTypeCombo();
}

void MainWindow::populateTypeCombo()
{
    QString sel = ui->widgetComboAdd_2->currentText();
    QStringList types;

    if (sel == "custom") {
        types = {
            "date","time","hour","hourhi","hourlo","min","minhi","minlo",
            "second","week","day","month","year",
            "calorie","distance","heartrate","redpoint","battery",
            "step","walk","exercise","icon","sleep","bluetooth","apm",
            "shortcut","anima","multimeter","gradient"
        };
    } else if (sel == "watch") {
        types = { "time" };
    } else if (sel == "ring" || sel == "progressbar") {
        types = { "battery","calorie","distance","heartrate","walk","exercise","step" };
    }

    ui->typeComboAdd_2->clear();
    ui->typeComboAdd_2->addItems(types);
}

// ─── Add Widget ───────────────────────────────────────────────────────────────
void MainWindow::onAddWidget()
{
    if (!projectOpen) return;

    QString widgetKind = ui->widgetComboAdd_2->currentText();
    QString typeVal    = ui->typeComboAdd_2->currentText();
    QJsonObject obj;

    if (widgetKind == "watch" && typeVal == "time") {

        // Line 414 - Hour hand
        ClockHandDialog hourDlg("hour", currentDevice.anchorX, currentDevice.anchorY, this);
        if (hourDlg.exec() != QDialog::Accepted) return;

        // Line 417 - Minute hand
        ClockHandDialog minDlg("minute", currentDevice.anchorX, currentDevice.anchorY, this);
        if (minDlg.exec() != QDialog::Accepted) return;

        // Line 420 - Second hand
        ClockHandDialog secDlg("second", currentDevice.anchorX, currentDevice.anchorY, this);
        if (secDlg.exec() != QDialog::Accepted) return;

        // Copy hand images into project folder
        auto copyHand = [&](const ClockHandDialog &dlg, const QString &destName) {
            QString src = dlg.filePath();
            if (src.isEmpty()) return;
            QString dst = projectDir + "/" + destName;
            if (QFile::exists(dst)) QFile::remove(dst);
            QFile::copy(src, dst);
        };

        QString hourFile = hourDlg.fileName().isEmpty() ? "hour.png"   : hourDlg.fileName();
        QString minFile  = minDlg.fileName().isEmpty()  ? "min.png"    : minDlg.fileName();
        QString secFile  = secDlg.fileName().isEmpty()  ? "second.png" : secDlg.fileName();

        copyHand(hourDlg, hourFile);
        copyHand(minDlg,  minFile);
        copyHand(secDlg,  secFile);

        // Build JSON — exact key order from spec
        obj["widget"]      = "watch";
        obj["type"]        = "time";
        obj["x"]           = 0;
        obj["y"]           = 0;
        obj["w"]           = currentDevice.canvasW;
        obj["h"]           = currentDevice.canvasH;
        obj["fgcolor"]     = "0xFFFFFFFF";
        obj["hour"]        = hourFile;
        obj["hourcenterx"] = hourDlg.centerX();
        obj["hourcentery"] = hourDlg.centerY();
        obj["houranchorx"] = hourDlg.anchorX();
        obj["houranchory"] = hourDlg.anchorY();
        obj["minute"]      = minFile;
        obj["mincenterx"]  = minDlg.centerX();
        obj["mincentery"]  = minDlg.centerY();
        obj["minanchorx"]  = minDlg.anchorX();
        obj["minanchory"]  = minDlg.anchorY();
        obj["second"]      = secFile;
        obj["seccenterx"]  = secDlg.centerX();
        obj["seccentery"]  = secDlg.centerY();
        obj["secanchorx"]  = secDlg.anchorX();
        obj["secanchory"]  = secDlg.anchorY();

    } else if (widgetKind == "custom") {
        // "Coming soon" types
        static const QSet<QString> comingSoon = {
            "multimeter","gradient","shortcut","sleep","bluetooth"
        };
        if (comingSoon.contains(typeVal)) {
            QMessageBox::information(this, "Coming Soon",
                                     QString("This \"%1\" type is coming soon.").arg(typeVal));
            return;
        }

        if (typeVal == "anima") {
            if (!addAnimaWidget()) return;
        } else if (typeVal == "redpoint") {
            if (!addRedpointWidget()) return;
        } else if (typeVal == "icon") {
            if (!addIconWidget()) return;
        } else {
            if (!addCustomWidget(typeVal)) return;
        }
        // After adding the widget, return to avoid creating a generic entry
        return;

    } else {
        // ring / progressbar — generic placeholder until rendering type confirmed
        obj["widget"] = widgetKind;
        obj["type"]   = typeVal;
        obj["x"]      = 0;
        obj["y"]      = 0;
        obj["w"]      = currentDevice.canvasW;
        obj["h"]      = currentDevice.canvasH;
    }

    // Only reaches here for generic widgets (ring, progressbar, etc.)
    WidgetEntry entry;
    entry.widgetType = widgetKind;
    entry.typeValue  = typeVal;
    entry.json       = obj;

    // Create a scene placeholder for non-watch widgets
    if (widgetKind != "watch") {
        int ex = obj["x"].toInt(), ey = obj["y"].toInt();
        int ew = obj["w"].toInt(), eh = obj["h"].toInt();

        QImage placeholder(ew > 0 ? ew : 320, eh > 0 ? eh : 20,
                           QImage::Format_ARGB32_Premultiplied);
        placeholder.fill(QColor(255, 255, 255, 40));
        QPainter pp(&placeholder);
        pp.setPen(QPen(QColor(255, 255, 0, 180), 1, Qt::DashLine));
        pp.drawRect(0, 0, placeholder.width() - 1, placeholder.height() - 1);
        pp.setPen(Qt::yellow);
        pp.setFont(QFont("Arial", 8));
        pp.drawText(4, 14, QString("%1/%2").arg(widgetKind, typeVal));
        pp.end();

        entry.sceneItem = new QGraphicsPixmapItem(QPixmap::fromImage(placeholder));
        entry.sceneItem->setPos(ex, ey);
        ui->graphicsView->scene()->addItem(entry.sceneItem);
    }
    widgetList.append(entry);

    updateJsonDisplay();
    rebuildCurrentWidgetCombo();

    int newIdx = widgetList.size() - 1;
    ui->currentWidgetCombo_2->blockSignals(true);
    ui->currentWidgetCombo_2->setCurrentIndex(newIdx);
    ui->currentWidgetCombo_2->blockSignals(false);
    selectWidget(newIdx);

    if (widgetKind == "watch") renderWatchHands();
}

// ─── Current Widget combo changed ─────────────────────────────────────────────
void MainWindow::onCurrentWidgetComboChanged(int index)
{
    selectWidget(index);
}

// ─── Select widget → update panel + scrollArea ────────────────────────────────
void MainWindow::selectWidget(int index)
{
    if (index < 0 || index >= widgetList.size()) {
        clearScrollArea();
        updateHighlight();
        return;
    }
    currentWidgetIndex = index;
    const WidgetEntry &e = widgetList[index];

    // currentTypeCombo_2
    ui->currentTypeCombo_2->blockSignals(true);
    ui->currentTypeCombo_2->clear();
    ui->currentTypeCombo_2->addItem(e.typeValue);
    ui->currentTypeCombo_2->setCurrentIndex(0);
    ui->currentTypeCombo_2->blockSignals(false);

    // X/Y/W/H spinboxes
    ui->xSpin_2->blockSignals(true);
    ui->ySpin_2->blockSignals(true);
    ui->wSpin_2->blockSignals(true);
    ui->hSpin_2->blockSignals(true);
    ui->xSpin_2->setValue(e.json["x"].toInt());
    ui->ySpin_2->setValue(e.json["y"].toInt());
    ui->wSpin_2->setValue(e.json.contains("w") ? e.json["w"].toInt() : currentDevice.canvasW);
    ui->hSpin_2->setValue(e.json.contains("h") ? e.json["h"].toInt() : currentDevice.canvasH);
    ui->xSpin_2->blockSignals(false);
    ui->ySpin_2->blockSignals(false);
    ui->wSpin_2->blockSignals(false);
    ui->hSpin_2->blockSignals(false);

    // scrollArea fields
    loadScrollAreaFromJson(e.json);

    // yellow highlight
    updateHighlight();
}

// ─── scrollArea: load from JSON ───────────────────────────────────────────────
//
// For every row in the field table:
//   • If the JSON object contains that key → check the checkbox, fill the lineEdit
//   • Otherwise → uncheck the checkbox, clear the lineEdit
//
void MainWindow::loadScrollAreaFromJson(const QJsonObject &obj)
{
    for (const FieldEntry &fe : scrollFields()) {
        QCheckBox *cb = findCB(this, fe.cbName);
        QLineEdit *le = findLE(this, fe.leName);
        if (!cb || !le) continue;

        if (obj.contains(fe.key)) {
            cb->setChecked(true);
            le->setEnabled(true);
            QJsonValue v = obj[fe.key];
            if (v.isString())
                le->setText(v.toString());
            else if (v.isBool())
                le->setText(v.toBool() ? "true" : "false");
            else
                le->setText(QString::number(v.toInt()));
        } else {
            cb->setChecked(false);
            le->setEnabled(false);
            le->clear();
        }
    }
}

// ─── scrollArea: clear all fields ─────────────────────────────────────────────
void MainWindow::clearScrollArea()
{
    for (const FieldEntry &fe : scrollFields()) {
        QCheckBox *cb = findCB(this, fe.cbName);
        QLineEdit *le = findLE(this, fe.leName);
        if (cb) cb->setChecked(false);
        if (le) { le->setEnabled(false); le->clear(); }
    }
}

// ─── scrollArea: read back into JSON ─────────────────────────────────────────
//
// Merges scrollArea fields into `base`. Checked = include key; unchecked = omit.
// For numeric-looking fields we store as int; otherwise string.
//
QJsonObject MainWindow::scrollAreaToJson(const QJsonObject &base) const
{
    // Keys that should always be stored as integers
    static const QSet<QString> intKeys = {
        "fontnum","numwidth","style","turn","time","frame","animabpp",
        "panchorx","panchory","pcenterx","pcentery",
        "startangle","endangle","pointer","ringedge","direction",
        "seccenterx","seccentery","secanchorx","secanchory",
        "mincenterx","mincentery","minanchorx","minanchory",
        "hourcenterx","hourcentery","houranchorx","houranchory"
    };

    QJsonObject result = base;

    for (const FieldEntry &fe : scrollFields()) {
        QCheckBox *cb = findCB(const_cast<MainWindow*>(this), fe.cbName);
        QLineEdit *le = findLE(const_cast<MainWindow*>(this), fe.leName);
        if (!cb || !le) continue;

        if (cb->isChecked()) {
            QString txt = le->text().trimmed();
            if (intKeys.contains(fe.key)) {
                bool ok;
                int iv = txt.toInt(&ok);
                result[fe.key] = ok ? QJsonValue(iv) : QJsonValue(txt);
            } else {
                result[fe.key] = txt;
            }
        } else {
            result.remove(fe.key);
        }
    }

    return result;
}

// ─── "Update Changes" button ──────────────────────────────────────────────────
void MainWindow::onUpdateChanges()
{
    if (currentWidgetIndex < 0) return;

    WidgetEntry &e = widgetList[currentWidgetIndex];

    // Commit spinbox values
    e.json["x"] = ui->xSpin_2->value();
    e.json["y"] = ui->ySpin_2->value();
    e.json["w"] = ui->wSpin_2->value();
    e.json["h"] = ui->hSpin_2->value();

    // Commit scrollArea fields
    e.json = scrollAreaToJson(e.json);

    // Rebuild the placeholder pixmap for generic widgets (size may have changed)
    if (e.widgetType != "watch" && e.sceneItem) {
        int ew = e.json["w"].toInt(); if (ew <= 0) ew = currentDevice.canvasW;
        int eh = e.json["h"].toInt(); if (eh <= 0) eh = 20;
        int ex = e.json["x"].toInt();
        int ey = e.json["y"].toInt();

        if (e.widgetType == "custom" && !e.imageStrip.isEmpty()) {
            // Re-render with updated position
            renderCustomWidget(currentWidgetIndex);
        } else {
            QImage placeholder(ew, eh, QImage::Format_ARGB32_Premultiplied);
            placeholder.fill(QColor(255, 255, 255, 40));
            QPainter pp(&placeholder);
            pp.setPen(QPen(QColor(255, 255, 0, 180), 1, Qt::DashLine));
            pp.drawRect(0, 0, ew - 1, eh - 1);
            pp.setPen(Qt::yellow);
            pp.setFont(QFont("Arial", 8));
            pp.drawText(4, 14, QString("%1/%2").arg(e.widgetType, e.typeValue));
            pp.end();

            e.sceneItem->setPixmap(QPixmap::fromImage(placeholder));
            e.sceneItem->setPos(ex, ey);
        }
    }

    updateHighlight();
    updateJsonDisplay();

    if (e.widgetType == "watch") renderWatchHands();
}

// ─── Real-time spinbox → JSON + scene position ────────────────────────────────
void MainWindow::onXSpinChanged(int v) {
    if (currentWidgetIndex < 0) return;
    WidgetEntry &e = widgetList[currentWidgetIndex];
    e.json["x"] = v;
    if (e.sceneItem) e.sceneItem->setPos(v, e.json["y"].toInt());
    updateHighlight();
    updateJsonDisplay();
}
void MainWindow::onYSpinChanged(int v) {
    if (currentWidgetIndex < 0) return;
    WidgetEntry &e = widgetList[currentWidgetIndex];
    e.json["y"] = v;
    if (e.sceneItem) e.sceneItem->setPos(e.json["x"].toInt(), v);
    updateHighlight();
    updateJsonDisplay();
}
void MainWindow::onWSpinChanged(int v) {
    if (currentWidgetIndex < 0) return;
    widgetList[currentWidgetIndex].json["w"] = v;
    updateJsonDisplay();
}
void MainWindow::onHSpinChanged(int v) {
    if (currentWidgetIndex < 0) return;
    widgetList[currentWidgetIndex].json["h"] = v;
    updateJsonDisplay();
}

// ─── Rebuild "Current Widget" combo ──────────────────────────────────────────
void MainWindow::rebuildCurrentWidgetCombo()
{
    ui->currentWidgetCombo_2->blockSignals(true);
    ui->currentWidgetCombo_2->clear();
    for (const auto &e : widgetList)
        ui->currentWidgetCombo_2->addItem(
            QString("%1 / %2").arg(e.widgetType, e.typeValue));
    ui->currentWidgetCombo_2->blockSignals(false);
}

// ─── JSON display ─────────────────────────────────────────────────────────────
void MainWindow::updateJsonDisplay()
{
    ui->plainTextEdit->setPlainText(prettyJson(buildRootJson()));
}

void MainWindow::updateFontJsonDisplay()
{
    QString result = "{\"item\":[";

    for (int i = 0; i < fontJsonItems.size(); ++i) {
        if (i > 0) result += ",";

        QJsonObject obj = fontJsonItems[i].toObject();

        // Start object
        result += "{";

        // Add fields in EXACT order: name, bpp, format
        result += "\"name\":\"" + obj["name"].toString() + "\"";
        result += ",\"bpp\":" + QString::number(obj["bpp"].toInt());
        result += ",\"format\":\"" + obj["format"].toString() + "\"";

        // Close object
        result += "}";
    }

    result += "]}";

    ui->plainTextEdit_2->setPlainText(result);
}

QJsonObject MainWindow::buildRootJson() const
{
    QJsonParseError err;
    QJsonDocument existing = QJsonDocument::fromJson(
        ui->plainTextEdit->toPlainText().toUtf8(), &err);

    QJsonObject root;
    if (!existing.isNull() && existing.isObject())
        root = existing.object();
    else {
        root["version"]          = 1;
        root["clouddialversion"] = 3;
        root["preview"]          = "preview.png";
        root["name"]             = "";
        root["author"]           = "";
        root["description"]      = "IDW20";
        root["deviceId"]         = "IDW20";
        root["bluetooth"]        = false;
        root["disturb"]          = false;
        root["battery"]          = false;
        root["compress"]         = "LZ4";
        root["bkground"]         = "";
    }

    QJsonArray items;
    for (const auto &e : widgetList)
        items.append(e.json);
    root["item"] = items;

    return root;
}

// ─── Pretty JSON ──────────────────────────────────────────────────────────────
QString MainWindow::prettyJson(const QJsonObject &root) const
{
    static const QStringList topKeys = {
        "version","clouddialversion","preview","name","author",
        "description","deviceId","bluetooth","disturb","battery",
        "compress","item","bkground"
    };
    static const QStringList watchKeys = {
        "widget","type","x","y","w","h","fgcolor",
        "hour","hourcenterx","hourcentery","houranchorx","houranchory",
        "minute","mincenterx","mincentery","minanchorx","minanchory",
        "second","seccenterx","seccentery","secanchorx","secanchory"
    };
    // Custom widget key order per spec
    static const QStringList customKeys = {
        "widget","type","x","y","w","h",
        "fgcolor","fgrender","align","metricinch","style",
        "font","fontnum"
    };
    // anima widget key order per spec
    static const QStringList animaKeys = {
        "widget","type","x","y","w","h",
        "time","turn","animatype","animaicon","frame","animabpp","animaformat"
    };
    // icon widget key order per spec
    static const QStringList iconKeys = {
        "widget","type","x","y","w","h",
        "bgcolor","bgrender","bg"
    };
    static const QStringList redpointKeys = {
        "widget","type","x","y","w","h",
        "font","fontnum"
    };
    static const QStringList genericKeys = { "widget","type","x","y","w","h" };

    auto valStr = [](const QJsonValue &v) -> QString {
        if (v.isString())  return "\"" + v.toString() + "\"";
        if (v.isBool())    return v.toBool() ? "true" : "false";
        if (v.isDouble())  return QString::number(v.toInt());
        return "null";
    };

    // Build ordered item objects respecting extra keys not in the static lists
    auto buildItemStr = [&](const QJsonObject &obj, int indent) -> QString {
        QString wType = obj["widget"].toString();

        // Start with the ordered template
        QString typeVal2 = obj["type"].toString();
        QStringList ordered;
        if (wType == "watch")                                    ordered = watchKeys;
        else if (wType == "custom" && typeVal2 == "anima")       ordered = animaKeys;
        else if (wType == "custom" && typeVal2 == "icon")        ordered = iconKeys;
        else if (wType == "custom" && typeVal2 == "redpoint")    ordered = redpointKeys;
        else if (wType == "custom")                              ordered = customKeys;
        else                                                     ordered = genericKeys;
        // Append any extra keys not already in the list, in JSON insertion order
        for (const QString &k : obj.keys())
            if (!ordered.contains(k)) ordered.append(k);

        QString pad(indent, ' ');
        QString inner;
        bool firstWritten = false;
        for (const QString &k : ordered) {
            if (!obj.contains(k)) continue;
            if (firstWritten) inner += ",\n";
            inner += pad + "    \"" + k + "\": " + valStr(obj[k]);
            firstWritten = true;
        }
        return pad + "{\n" + inner + "\n" + pad + "}";
    };

    QString out = "{\n";
    for (int ki = 0; ki < topKeys.size(); ++ki) {
        const QString &k = topKeys[ki];
        if (!root.contains(k)) continue;

        if (k == "item") {
            QJsonArray arr = root["item"].toArray();
            out += "    \"item\": ";
            if (arr.isEmpty()) {
                out += "[]";
            } else {
                out += "[\n";
                for (int ai = 0; ai < arr.size(); ++ai) {
                    out += buildItemStr(arr[ai].toObject(), 8);
                    if (ai < arr.size() - 1) out += ",";
                    out += "\n";
                }
                out += "    ]";
            }
        } else {
            out += "    \"" + k + "\": " + valStr(root[k]);
        }

        // Trailing comma if more keys follow
        bool hasMore = false;
        for (int kj = ki + 1; kj < topKeys.size(); ++kj)
            if (root.contains(topKeys[kj])) { hasMore = true; break; }
        if (hasMore) out += ",";
        out += "\n";
    }
    out += "}";
    return out;
}

// ─── Render watch hands ───────────────────────────────────────────────────────
//
// Angle formulas taken directly from Wf_Editor_For_IDW20.py Renderer.render():
//
//   hour   angle = (hour % 12 + minute / 60.0) * 30.0
//   minute angle = (minute + second / 60.0)    *  6.0
//   second angle = second                       *  6.0
//
// Rendering mirrors _paste_centered():
//   1. Expand the image into a square canvas centred on (cx, cy)
//   2. Rotate that canvas by -angle (Qt rotates CCW for positive degrees,
//      so we negate to get clockwise clock rotation)
//   3. Place the rotated canvas so its centre lands at (anchorx, anchory)
//
void MainWindow::renderWatchHands()
{
    // Find last watch widget
    for (int i = widgetList.size() - 1; i >= 0; --i) {
        if (widgetList[i].widgetType != "watch") continue;

        const QJsonObject &w = widgetList[i].json;
        QGraphicsScene *scene = ui->graphicsView->scene();

        auto removeItem = [&](QGraphicsPixmapItem *&item) {
            if (item) { scene->removeItem(item); delete item; item = nullptr; }
        };
        removeItem(hourHandItem);
        removeItem(minHandItem);
        removeItem(secHandItem);

        // Angles from .py
        double hourAngle = (PREVIEW_HOUR % 12 + PREVIEW_MIN / 60.0) * 30.0;
        double minAngle  = (PREVIEW_MIN  + PREVIEW_SEC / 60.0)       *  6.0;
        double secAngle  =  PREVIEW_SEC                               *  6.0;

        // _paste_centered equivalent in Qt:
        // The Python big-canvas approach: pad symmetrically around (ox,oy),
        // rotate by -angle (PIL rotate is CCW; -angle makes it CW for clock hands),
        // then paste so big canvas centre = (anchorx, anchory).
        //
        // In Qt we replicate the same geometry:
        auto pasteHand = [&](const QString &imgKey,
                             const QString &cxKey, const QString &cyKey,
                             const QString &axKey, const QString &ayKey,
                             double angleDeg,
                             QGraphicsPixmapItem *&itemRef)
        {
            QString file = projectDir + "/" + w[imgKey].toString();
            QImage src(file);
            if (src.isNull()) return;

            int ox = w[cxKey].toInt();   // pivot within source image
            int oy = w[cyKey].toInt();
            int ax = w[axKey].toInt();   // pivot position on watch face
            int ay = w[ayKey].toInt();

            // Build symmetric padding canvas (same as Python)
            int padL = std::max(ox, src.width()  - ox);
            int padT = std::max(oy, src.height() - oy);
            int bigW = padL * 2;
            int bigH = padT * 2;

            QImage big(bigW, bigH, QImage::Format_ARGB32_Premultiplied);
            big.fill(Qt::transparent);
            QPainter p(&big);
            p.drawImage(padL - ox, padT - oy, src);
            p.end();

            // Rotate around big-canvas centre by -angleDeg (CW for positive angle)
            QTransform t;
            t.translate(bigW / 2.0, bigH / 2.0);
            t.rotate(angleDeg);           // CW in Qt = matches PIL's big.rotate(-angle) which is also CW
            t.translate(-bigW / 2.0, -bigH / 2.0);

            QImage rotated = big.transformed(t, Qt::SmoothTransformation);

            // Place so rotated-canvas centre lands at (ax, ay) on the scene
            QPixmap pm = QPixmap::fromImage(rotated);
            itemRef = new QGraphicsPixmapItem(pm);
            itemRef->setPos(ax - rotated.width()  / 2.0,
                            ay - rotated.height() / 2.0);
            scene->addItem(itemRef);
        };

        pasteHand("hour",   "hourcenterx","hourcentery","houranchorx","houranchory", hourAngle, hourHandItem);
        pasteHand("minute", "mincenterx", "mincentery", "minanchorx", "minanchory",  minAngle,  minHandItem);
        pasteHand("second", "seccenterx", "seccentery", "secanchorx", "secanchory",  secAngle,  secHandItem);

        // Shift the whole watch by the widget's x/y position
        int wx = w["x"].toInt(), wy = w["y"].toInt();
        if (wx != 0 || wy != 0) {
            auto shift = [&](QGraphicsPixmapItem *item) {
                if (item) item->setPos(item->pos() + QPointF(wx, wy));
            };
            shift(hourHandItem);
            shift(minHandItem);
            shift(secHandItem);
        }
        break;
    }
}

// ─── Background upload ────────────────────────────────────────────────────────
void MainWindow::onUploadBkgroundImg()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Bkground Image"), "", tr("Image Files (*.png *.bmp)"));
    if (fileName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    QString suffix = fileInfo.suffix().toLower();
    if (suffix != "png" && suffix != "bmp") {
        QMessageBox::critical(this, tr("Invalid File Type"),
                              tr("Only PNG and BMP files are supported."));
        return;
    }

    QImage image(fileName);
    if (image.isNull()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load the image file."));
        return;
    }

    // Copy to project folder as files0.png, files1.png, …
    QString destName = projectOpen
                           ? QString("files%1.%2").arg(fileCounter++).arg(suffix)
                           : fileInfo.fileName();

    if (projectOpen) {
        QString dst = projectDir + "/" + destName;
        if (QFile::exists(dst)) QFile::remove(dst);
        QFile::copy(fileName, dst);
    }

    // Show on canvas
    QGraphicsScene *scene = ui->graphicsView->scene();
    scene->addItem(new QGraphicsPixmapItem(QPixmap::fromImage(image)));

    // Update bkground in JSON
    QJsonParseError perr;
    QJsonDocument doc = QJsonDocument::fromJson(
        ui->plainTextEdit->toPlainText().toUtf8(), &perr);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject root = doc.object();
        root["bkground"] = destName;
        ui->plainTextEdit->setPlainText(prettyJson(root));
    }
}

// ─── Background corner matcher ────────────────────────────────────────────────
void MainWindow::pushButtonEditBkgroundPic()
{
    if (!widgetListWindow) {
        widgetListWindow = new bkground_helper(nullptr);
        widgetListWindow->setWindowTitle("Bkground Corner Matcher");
        widgetListWindow->setWindowFlags(Qt::Window);
        widgetListWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(widgetListWindow, &QObject::destroyed, this, [this]() {
            widgetListWindow = nullptr;
        });
    }
    widgetListWindow->show();
    widgetListWindow->raise();
    widgetListWindow->activateWindow();
}

// ─── Save iwf.json ────────────────────────────────────────────────────────────
// Uses the same prettyJson() helper (4-space indent, ordered keys) used
// everywhere else in the editor.
void MainWindow::onSaveIwfJson()
{
    if (!projectOpen) {
        QMessageBox::warning(this, "No Project", "Please create a project first.");
        return;
    }

    QString path = projectDir + "/iwf.json";
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error",
                              "Could not write iwf.json to:\n" + path);
        return;
    }

    file.write(prettyJson(buildRootJson()).toUtf8());
    file.close();

    QMessageBox::information(this, "Saved",
                             "Successfully saved iwf.json to \"" + projectDir + "\"");
}

// ─── Save font.json ───────────────────────────────────────────────────────────
// font.json is always compact (no indentation), matching the Python editor's
// json.dumps(..., separators=(',', ':')) output.
void MainWindow::onSaveFontJson()
{
    if (!projectOpen) {
        QMessageBox::warning(this, "No Project", "Please create a project first.");
        return;
    }

    QString path = projectDir + "/font.json";
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error",
                              "Could not write font.json to:\n" + path);
        return;
    }

    // Parse whatever the user has typed in the font.json plainTextEdit
    QJsonParseError perr;
    QJsonDocument doc = QJsonDocument::fromJson(
        ui->plainTextEdit_2->toPlainText().toUtf8(), &perr);

    if (doc.isNull()) {
        QMessageBox::critical(this, "Invalid JSON",
                              "font.json contains invalid JSON:\n" + perr.errorString());
        return;
    }

    // Manually build the JSON string to preserve exact key order
    QJsonObject root = doc.object();
    QString result = "{";

    // Add "item" array first (or whatever order you want)
    if (root.contains("item")) {
        result += "\"item\":[";

        QJsonArray items = root["item"].toArray();
        for (int i = 0; i < items.size(); ++i) {
            if (i > 0) result += ",";

            QJsonObject item = items[i].toObject();
            result += "{";

            // Add keys in EXACT order: name, bpp, format
            bool first = true;

            if (item.contains("name")) {
                if (!first) result += ",";
                result += "\"name\":\"" + item["name"].toString() + "\"";
                first = false;
            }

            if (item.contains("bpp")) {
                if (!first) result += ",";
                result += "\"bpp\":" + QString::number(item["bpp"].toInt());
                first = false;
            }

            if (item.contains("format")) {
                if (!first) result += ",";
                result += "\"format\":\"" + item["format"].toString() + "\"";
                first = false;
            }

            // Add any other keys that might exist
            for (const QString &key : item.keys()) {
                if (key != "name" && key != "bpp" && key != "format") {
                    if (!first) result += ",";

                    QJsonValue val = item[key];
                    if (val.isString()) {
                        result += "\"" + key + "\":\"" + val.toString() + "\"";
                    } else if (val.isBool()) {
                        result += "\"" + key + "\":" + (val.toBool() ? "true" : "false");
                    } else if (val.isDouble()) {
                        result += "\"" + key + "\":" + QString::number(val.toInt());
                    }
                    first = false;
                }
            }

            result += "}";
        }

        result += "]";
    }

    result += "}";

    file.write(result.toUtf8());
    file.close();

    QMessageBox::information(this, "Saved",
                             "Successfully saved font.json to \"" + projectDir + "\"");
}

// ─── Save Preview (pushButtonCreerPreview_2) ──────────────────────────────────
//
// Output canvas: 272 × 324 px, black background.
// Layer order (back → front):
//   1. Black fill                         — 272×324, full size
//   2. Watch face (graphicsView scene)    — scaled to 0.97× of output, centred
//   3. border.png (:/images/border.png)   — 1× scale (272×324), composited on top
//
void MainWindow::onSavePreview()
{
    if (!projectOpen) {
        QMessageBox::warning(this, "No Project", "Please create a project first.");
        return;
    }

    const int OUT_W     = currentDevice.previewW;
    const int OUT_H     = currentDevice.previewH;
    const double SCALE = currentDevice.previewScale;

    // ── Collect all redpoint widget scene items ────────────────────────────────
    QList<QGraphicsItem*> redpointItems;
    for (const WidgetEntry &e : widgetList) {
        if (e.widgetType == "custom" && e.typeValue == "redpoint" && e.sceneItem) {
            redpointItems.append(e.sceneItem);
        }
    }

    // ── Temporarily hide the redpoint widgets ──────────────────────────────────
    for (QGraphicsItem *item : redpointItems) {
        item->hide();
    }

    // ── Temporarily hide the yellow highlight ─────────────────────────────────
    bool highlightWasVisible = (selectionHighlight != nullptr);
    if (highlightWasVisible) {
        selectionHighlight->hide();
    }

    // 1. Render scene on an opaque base (Format_RGB888)
    QGraphicsScene *scene = ui->graphicsView->scene();
    QImage sceneImg(currentDevice.canvasW, currentDevice.canvasH, QImage::Format_RGB888);
    sceneImg.fill(Qt::black);
    {
        QPainter sp(&sceneImg);
        sp.setRenderHint(QPainter::SmoothPixmapTransform);
        sp.setRenderHint(QPainter::Antialiasing);
        scene->render(&sp, QRectF(0, 0, currentDevice.canvasW, currentDevice.canvasH), scene->sceneRect());
    }

    // ── Restore the yellow highlight ─────────────────────────────────────────
    if (highlightWasVisible) {
        selectionHighlight->show();
    }

    // ── Restore the redpoint widgets ──────────────────────────────────────────
    for (QGraphicsItem *item : redpointItems) {
        item->show();
    }

    // 2. Scale scene according to previewScale
    int scaledW = qRound(OUT_W * SCALE);
    int scaledH = qRound(OUT_H * SCALE);
    QImage scaledScene = sceneImg.scaled(scaledW, scaledH,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);

    // 3. Composite fully opaque final preview (Format_RGB888)
    QImage final(OUT_W, OUT_H, QImage::Format_RGB888);
    final.fill(Qt::black); // Opaque background instead of Qt::transparent

    QPainter p(&final);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // Create clipped background based on corner radius
    QPainterPath clipPath;
    clipPath.addRoundedRect(QRectF(0, 0, OUT_W, OUT_H),
                            currentDevice.previewCornerRadius,
                            currentDevice.previewCornerRadius);
    p.setClipPath(clipPath);
    p.fillRect(0, 0, OUT_W, OUT_H, Qt::black);

    // Center scaled canvas within preview view
    int offX = (OUT_W - scaledW) / 2;
    int offY = (OUT_H - scaledH) / 2;
    p.drawImage(offX, offY, scaledScene);

    // Remove clipping mask to draw outer border cleanly
    p.setClipping(false);

    // Draw procedural vector border overlay
    QPen borderPen(currentDevice.previewBorderColor, currentDevice.previewBorderWidth);
    p.setPen(borderPen);
    p.setBrush(Qt::NoBrush);

    double rectX = (OUT_W - currentDevice.previewBorderRectWidth) / 2.0;
    double rectY = (OUT_H - currentDevice.previewBorderRectHeight) / 2.0;

    QRectF borderRect(rectX, rectY,
                      currentDevice.previewBorderRectWidth,
                      currentDevice.previewBorderRectHeight);

    p.drawRoundedRect(borderRect,
                      currentDevice.previewCornerRadius,
                      currentDevice.previewCornerRadius);
    p.end();

    // ── 4. Save to project directory as preview.png ──────────────────────────
    QString outPath = projectDir + "/preview.png";
    if (!final.save(outPath, "PNG")) {
        QMessageBox::critical(this, "Error",
                              "Could not save preview to:\n" + outPath);
        return;
    }

    QMessageBox::information(this, "Saved",
                             "Successfully saved preview to \"" + projectDir + "\"");
}

// ─── Yellow selection highlight ───────────────────────────────────────────────
// Draws a 2px yellow rectangle over the selected widget's w×h bounding box.
// Watch widgets use the full 320×385 scene area.
void MainWindow::updateHighlight()
{
    QGraphicsScene *scene = ui->graphicsView->scene();

    // Remove old highlight
    if (selectionHighlight) {
        scene->removeItem(selectionHighlight);
        delete selectionHighlight;
        selectionHighlight = nullptr;
    }

    if (currentWidgetIndex < 0 || currentWidgetIndex >= widgetList.size())
        return;

    const WidgetEntry &e = widgetList[currentWidgetIndex];
    int x = e.json["x"].toInt();
    int y = e.json["y"].toInt();
    int w = e.json.contains("w") ? e.json["w"].toInt() : currentDevice.canvasW;
    int h = e.json.contains("h") ? e.json["h"].toInt() : currentDevice.canvasH;

    selectionHighlight = scene->addRect(x, y, w, h,
                                        QPen(QColor(255, 220, 0), 2, Qt::SolidLine),
                                        QBrush(Qt::NoBrush));
    selectionHighlight->setZValue(1000); // always on top
}

// ─── Custom widget: dialog + image loading ────────────────────────────────────
//
// Digit-type widgets (numbers): filenames are 0.png … 9.png plus colon.png,
//   slash.png, period.png, percent.png, dash.png as needed.
// Letter-type widgets:
//   week : en_sun, en_mon, en_tue, en_wed, en_thur, en_fri, en_sat
//   month: en_jan, en_feb, en_mar, en_apr, en_may, en_june, en_july,
//           en_aug, en_sept, en_oct, en_nov, en_dec
//   apm  : en_am, en_pm
//
bool MainWindow::addCustomWidget(const QString &typeVal)
{
    // ── 1. Ask for folder name ────────────────────────────────────────────────
    bool ok;
    QString folderName = QInputDialog::getText(
        this, "Font Folder Name",
        "Enter the folder name for the widget images\n"
        "(will be created inside the project directory):",
        QLineEdit::Normal, typeVal, &ok);
    if (!ok || folderName.trimmed().isEmpty()) return false;
    folderName = folderName.trimmed();

    // ── 2. Ask user to select image files ────────────────────────────────────
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        QString("Select images for \"%1\" widget (folder: %2)")
            .arg(typeVal, folderName),
        "", "Images (*.png *.bmp)");
    if (files.isEmpty()) return false;

    // ── 3. Create destination folder and copy files ───────────────────────────
    QString destFolder = projectDir + "/" + folderName;
    QDir().mkpath(destFolder);

    QMap<QString, QImage> strip;
    int imageCount = 0;
    QString ext;

    for (const QString &src : files) {
        QFileInfo fi(src);
        QString baseName = fi.completeBaseName(); // e.g. "0", "en_wed"
        QString suffix   = fi.suffix().toLower();
        if (ext.isEmpty()) ext = suffix;

        QString dst = destFolder + "/" + fi.fileName();
        if (QFile::exists(dst)) QFile::remove(dst);
        QFile::copy(src, dst);

        QImage img(src);
        if (!img.isNull()) {
            strip[baseName] = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            imageCount++;
        }
    }

    // ── 4. Determine preview value string for this type ───────────────────────
    // digit-based types
    static const QMap<QString, QString> previewValues = {
                                                         {"time",     "10:08"},
                                                         {"hour",     "10"},
                                                         {"hourhi",   "1"},
                                                         {"hourlo",   "0"},
                                                         {"min",      "08"},
                                                         {"minhi",    "0"},
                                                         {"minlo",    "8"},
                                                         {"second",   "36"},
                                                         {"date",     "24/09"},
                                                         {"day",      "24"},
                                                         {"year",     "2025"},
                                                         {"step",     "23980"},
                                                         {"calorie",  "839"},
                                                         {"heartrate","128"},
                                                         {"distance", "16.79"},
                                                         {"exercise", "20"},
                                                         {"walk",     "10"},
                                                         {"battery",  "100%"},
                                                         {"weather",  "28"},   // rendered without unit; unit comes from style
                                                         // letter-based
                                                         {"week",     "en_wed"},
                                                         {"month",    "en_sept"},
                                                         {"apm",      "en_am"},
                                                         };
    QString previewVal = previewValues.value(typeVal, "0");

    // ── 5. Measure auto-width/height from rendered preview ────────────────────
    // Build a temporary entry to measure
    WidgetEntry tempEntry;
    tempEntry.widgetType = "custom";
    tempEntry.typeValue  = typeVal;
    tempEntry.imageStrip = strip;
    tempEntry.fontFolder = folderName;

    // Build the JSON with correct key order per spec
    QJsonObject obj;
    obj["widget"]   = "custom";
    obj["type"]     = typeVal;
    obj["x"]        = 0;
    obj["y"]        = 0;
    obj["fgcolor"]  = "0xFFFFFFFF";
    obj["fgrender"] = "0xFFFFFFFF";
    obj["align"]    = "left";

    if (typeVal == "distance") obj["metricinch"] = 1;
    if (typeVal == "weather")  obj["style"]      = 2;
    if (typeVal == "date")  obj["style"]      = 1;
    if (typeVal == "week") obj["style"] = 0;
    if (typeVal == "month") obj["style"] = 0;

    obj["font"]    = folderName;
    obj["fontnum"] = imageCount;

    tempEntry.json = obj;

    // Measure by rendering with preview value
    QSize sz = measureCustomWidget(tempEntry);
    obj["w"] = sz.width()  > 0 ? sz.width()  : 50;
    obj["h"] = sz.height() > 0 ? sz.height() : 20;
    tempEntry.json = obj;

    // ── 6. Update font.json ───────────────────────────────────────────────────
    // Add entry if not already present (matched by name)
    bool fontExists = false;
    for (int fi = 0; fi < fontJsonItems.size(); ++fi) {
        if (fontJsonItems[fi].toObject()["name"].toString() == folderName) {
            fontExists = true; break;
        }
    }
    if (!fontExists) {
        QJsonObject fontItem;
        fontItem["name"]   = folderName;
        fontItem["bpp"]    = 16;
        fontItem["format"] = ext.isEmpty() ? "png" : ext;
        fontJsonItems.append(fontItem);
        updateFontJsonDisplay();
    }

    // ── 7. Add to widget list and render ─────────────────────────────────────
    widgetList.append(tempEntry);
    int newIdx = widgetList.size() - 1;
    renderCustomWidget(newIdx);

    updateJsonDisplay();
    rebuildCurrentWidgetCombo();

    ui->currentWidgetCombo_2->blockSignals(true);
    ui->currentWidgetCombo_2->setCurrentIndex(newIdx);
    ui->currentWidgetCombo_2->blockSignals(false);
    selectWidget(newIdx);

    return true;
}

// ─── Measure auto w/h by dry-running the renderer ────────────────────────────
QSize MainWindow::measureCustomWidget(const WidgetEntry &e) const
{
    QImage img = renderCustomWidgetImage(e);
    if (img.isNull()) return QSize(50, 20);
    return img.size();
}

// ─── Render a custom widget to a QImage ──────────────────────────────────────
//
// Letter widgets (week, month, apm): look up the single named image.
// Digit widgets: iterate the preview value string character by character,
//   look up each glyph, composite left-to-right.
//
QImage MainWindow::renderCustomWidgetImage(const WidgetEntry &e) const
{
    const QString &typeVal = e.typeValue;
    const QMap<QString, QImage> &strip = e.imageStrip;

    // ── anima widget: display first frame as static image ────────────────────
    if (typeVal == "anima") {
        // Frames are named 0.png, 1.png … pick frame 0 as static preview
        if (strip.contains("0")) return strip["0"];
        if (!strip.isEmpty())    return strip.first();
        return QImage();
    }

    // ── icon widget: display bg image as static image ─────────────────────────
    if (typeVal == "icon") {
        if (strip.contains("__icon__")) return strip["__icon__"];
        if (!strip.isEmpty())           return strip.first();
        return QImage();
    }

    // ── letter widgets ────────────────────────────────────────────────────────
    static const QSet<QString> letterTypes = { "week", "month", "apm" };
    if (letterTypes.contains(typeVal)) {
        static const QMap<QString, QString> letterPreview = {
                                                             {"week",  "en_wed"},
                                                             {"month", "en_sept"},
                                                             {"apm",   "en_am"},
                                                             };
        QString key = letterPreview.value(typeVal, "");
        if (strip.contains(key)) return strip[key];
        // fallback: return first image found
        if (!strip.isEmpty()) return strip.first();
        return QImage();
    }

    // ── digit widgets ─────────────────────────────────────────────────────────
    static const QMap<QString, QString> digitPreview = {
                                                        {"time",     "10:08"},
                                                        {"hour",     "10"},
                                                        {"hourhi",   "1"},
                                                        {"hourlo",   "0"},
                                                        {"min",      "08"},
                                                        {"minhi",    "0"},
                                                        {"minlo",    "8"},
                                                        {"second",   "36"},
                                                        {"date",     "24/09"},
                                                        {"day",      "24"},
                                                        {"year",     "2025"},
                                                        {"step",     "23980"},
                                                        {"calorie",  "839"},
                                                        {"heartrate","128"},
                                                        {"distance", "16.79"},
                                                        {"exercise", "20"},
                                                        {"walk",     "10"},
                                                        {"battery",  "100%"},
                                                        {"weather",  "28"},
                                                        };
    QString value = digitPreview.value(typeVal, "0");

    // Special character → numbered image filename mapping:
    //   10 = colon (:) / slash (/) / percent (%) / period (.) / dash (-)
    //   11 = degree-Celsius  (°C, weather widget, style=2)
    //   12 = degree-Fahrenheit (°F, weather widget, style other)
    static const QMap<QChar, QString> specialMap = {
        {QChar(':'),    "10"},
        {QChar('/'),    "10"},
        {QChar('%'),    "10"},
        {QChar('.'),    "10"},
        {QChar('-'),    "10"},
        {QChar(0x00B0), "10"}, // bare degree sign °
    };

    // For the weather widget, append the unit suffix
    QString renderValue = value;
    if (typeVal == "weather") {
        int style = e.json.contains("style") ? e.json["style"].toInt() : 2;
        renderValue += (style == 2) ? QChar(0xE001) : QChar(0xE002);
    }

    // Extended map for weather unit sentinels
    static const QMap<QChar, QString> weatherMap = {
        {QChar(0xE001), "11"}, // °C
        {QChar(0xE002), "12"}, // °F
    };

    // Collect glyph images in order
    struct Glyph { const QImage *img; int w; int h; };
    QVector<Glyph> glyphs;
    int totalW = 0, maxH = 0;

    for (QChar ch : renderValue) {
        QString key;
        if (ch.isDigit())
            key = QString(ch);
        else if (weatherMap.contains(ch))
            key = weatherMap[ch];
        else if (specialMap.contains(ch))
            key = specialMap[ch];
        else
            continue;

        auto it = strip.find(key);
        if (it != strip.end()) {
            const QImage *img = &it.value();
            if (!img->isNull()) {
                glyphs.append({img, img->width(), img->height()});
                totalW += img->width();
                maxH = std::max(maxH, img->height());
            }
        }
    }

    if (glyphs.isEmpty()) {
        return QImage();
    }

    // ✨ Get alignment from JSON (default to "left")
    QString align = e.json.value("align").toString("left");

    // Canvas width: use the widget's w field if set and wider than the glyph
    // strip, so alignment has room to work. Otherwise just use totalW.
    int canvasW = std::max(totalW, e.json.contains("w") ? e.json["w"].toInt() : 0);
    if (canvasW <= 0) canvasW = totalW;

    // Calculate x offset for alignment (mirrors the Python logic exactly)
    int xpos = 0;
    if (align == "center")
        xpos = (canvasW - totalW) / 2;
    else if (align == "right")
        xpos = canvasW - totalW;
    // "left" → xpos stays 0

    // Create the final image with proper alignment
    QImage result(canvasW, maxH, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    int x = xpos;
    for (const Glyph &g : glyphs) {
        painter.drawImage(x, 0, *g.img);
        x += g.w;
    }

    return result;
}

// ─── Place / refresh a custom widget's scene item ────────────────────────────
void MainWindow::renderCustomWidget(int index, bool preserveSize)
{
    if (index < 0 || index >= widgetList.size()) return;
    WidgetEntry &e = widgetList[index];

    QGraphicsScene *scene = ui->graphicsView->scene();
    if (e.sceneItem) {
        scene->removeItem(e.sceneItem);
        delete e.sceneItem;
        e.sceneItem = nullptr;
    }

    QImage rendered = renderCustomWidgetImage(e);

    QImage display;
    if (rendered.isNull()) {
        // Fallback: plain placeholder
        int ew = std::max(e.json["w"].toInt(), 50);
        int eh = std::max(e.json["h"].toInt(), 20);
        display = QImage(ew, eh, QImage::Format_ARGB32_Premultiplied);
        display.fill(QColor(255, 255, 255, 40));
        QPainter pp(&display);
        pp.setPen(QPen(QColor(255, 255, 0, 180), 1, Qt::DashLine));
        pp.drawRect(0, 0, ew - 1, eh - 1);
        pp.setPen(Qt::yellow);
        pp.setFont(QFont("Arial", 8));
        pp.drawText(4, 14, QString("custom/%1").arg(e.typeValue));
        pp.end();
    } else {
        display = rendered;
        // preserveSize=true when loading from existing JSON — keep original w/h
        // so alignment offsets computed from them stay correct.
        if (!preserveSize) {
            e.json["w"] = rendered.width();
            e.json["h"] = rendered.height();
        }
    }

    e.sceneItem = new QGraphicsPixmapItem(QPixmap::fromImage(display));
    e.sceneItem->setPos(e.json["x"].toInt(), e.json["y"].toInt());
    scene->addItem(e.sceneItem);
}

// ─── Load image strip from a folder on disk ───────────────────────────────────
// Loads all PNG/BMP files in `folder`, keyed by filename stem (e.g. "0", "10",
// "en_wed"). Used both when opening an existing iwf.json and when importing new
// custom widget images.
QMap<QString, QImage> MainWindow::loadImageStrip(const QString &folder) const
{
    QMap<QString, QImage> strip;
    QDir dir(folder);
    if (!dir.exists()) return strip;

    const QStringList filters = { "*.png", "*.bmp", "*.PNG", "*.BMP" };
    for (const QString &fileName : dir.entryList(filters, QDir::Files)) {
        QFileInfo fi(fileName);
        QImage img(dir.filePath(fileName));
        if (!img.isNull())
            strip[fi.completeBaseName()] =
                img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }
    return strip;
}

// ─── Open iwf.json ────────────────────────────────────────────────────────────
//
// Supported widget types that are fully loaded and rendered:
//   watch/time  — clock hands (hour, minute, second)
//   custom/*    — digit and letter image widgets (all non-"coming soon" types)
//
// Unsupported / coming-soon types are silently skipped with a note in the
// status bar at the end.
//
void MainWindow::onOpenIwfJson()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Open iwf.json", QString(), "JSON files (*.json)");
    if (path.isEmpty()) return;

    // ── Parse JSON ────────────────────────────────────────────────────────────
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file:\n" + path);
        return;
    }
    QJsonParseError perr;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &perr);
    f.close();

    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(this, "Invalid JSON",
                              "Could not parse iwf.json:\n" + perr.errorString());
        return;
    }

    QJsonObject root = doc.object();

    QString devId = root["deviceId"].toString();
    if (!devId.isEmpty()) {
        int idx = ui->comboBox->findText(devId);
        if (idx != -1) {
            ui->comboBox->setCurrentIndex(idx);
        }
        applyDeviceConfig(devId);
    }

    // ── Set project directory = folder containing the JSON ───────────────────
    projectDir  = QFileInfo(path).absolutePath();
    projectOpen = true;
    fileCounter = 0;

    // ── Clear existing state ──────────────────────────────────────────────────
    QGraphicsScene *scene = ui->graphicsView->scene();
    scene->clear();
    hourHandItem       = nullptr;
    minHandItem        = nullptr;
    secHandItem        = nullptr;
    selectionHighlight = nullptr;
    widgetList.clear();
    fontJsonItems = QJsonArray();
    currentWidgetIndex = -1;

    // ── Display raw JSON in text panel ────────────────────────────────────────
    ui->plainTextEdit->setPlainText(prettyJson(root));

    // ── Load background image ─────────────────────────────────────────────────
    QString bkground = root["bkground"].toString();
    if (!bkground.isEmpty()) {
        QString bgPath = projectDir + "/" + bkground;
        QImage bgImg(bgPath);
        if (!bgImg.isNull()) {
            scene->addItem(new QGraphicsPixmapItem(
                QPixmap::fromImage(bgImg)));
        }
    }

    // ── Coming-soon set (same as addCustomWidget guard) ───────────────────────
    static const QSet<QString> comingSoon = {
        "multimeter","gradient",
        "shortcut","sleep","bluetooth"
    };

    // ── Supported custom types (rendered with image strip) ────────────────────
    static const QSet<QString> supportedCustom = {
        "date","time","hour","hourhi","hourlo","min","minhi","minlo",
        "second","week","day","month","year",
        "calorie","distance","heartrate","battery",
        "step","walk","exercise","apm","weather",
        "anima","icon","redpoint"
    };

    int skipped = 0;

    // ── Iterate items ─────────────────────────────────────────────────────────
    for (const QJsonValue &val : root["item"].toArray()) {
        QJsonObject obj = val.toObject();
        QString widgetKind = obj["widget"].toString();
        QString typeVal    = obj["type"].toString();

        // ── watch/time ────────────────────────────────────────────────────────
        if (widgetKind == "watch" && typeVal == "time") {
            WidgetEntry entry;
            entry.widgetType = "watch";
            entry.typeValue  = "time";
            entry.json       = obj;
            widgetList.append(entry);
            // renderWatchHands() reads from the last watch entry
            renderWatchHands();
            continue;
        }

        // ── custom/anima ──────────────────────────────────────────────────────
        if (widgetKind == "custom" && typeVal == "anima") {
            // Load first frame from the animaicon folder as a static preview
            QString animaIcon   = obj["animaicon"].toString();
            QString folderPath  = projectDir + "/" + animaIcon;
            QMap<QString, QImage> strip = loadImageStrip(folderPath);

            WidgetEntry entry;
            entry.widgetType = "custom";
            entry.typeValue  = "anima";
            entry.json       = obj;
            entry.fontFolder = animaIcon;
            entry.imageStrip = strip;

            int idx = widgetList.size();
            widgetList.append(entry);
            renderCustomWidget(idx, true);

            // Register in font.json (ext from animaformat field or disk)
            QString animaExt = obj["animaformat"].toString().toLower();
            if (animaExt.isEmpty()) {
                QDir d(folderPath);
                QStringList found = d.entryList({"*.png","*.bmp","*.PNG","*.BMP"}, QDir::Files);
                if (!found.isEmpty())
                    animaExt = QFileInfo(found.first()).suffix().toLower();
            }
            if (animaExt.isEmpty()) animaExt = "png";

            bool animaFound = false;
            for (const QJsonValue &fv : fontJsonItems)
                if (fv.toObject()["name"].toString() == animaIcon)
                { animaFound = true; break; }
            if (!animaFound && !animaIcon.isEmpty()) {
                QJsonObject fi;
                fi["name"]   = animaIcon;
                fi["bpp"]    = 16;
                fi["format"] = animaExt;
                fontJsonItems.append(fi);
            }
            continue;
        }

        // ── custom/icon ───────────────────────────────────────────────────────
        if (widgetKind == "custom" && typeVal == "icon") {
            QString bgFile   = obj["bg"].toString();
            QString imgPath  = projectDir + "/" + bgFile;
            QImage img(imgPath);

            WidgetEntry entry;
            entry.widgetType = "custom";
            entry.typeValue  = "icon";
            entry.json       = obj;
            entry.fontFolder = bgFile;   // reuse fontFolder to store filename
            if (!img.isNull())
                entry.imageStrip["__icon__"] =
                    img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

            int idx = widgetList.size();
            widgetList.append(entry);
            renderCustomWidget(idx, true);
            continue;
        }

        // ── custom (digit / letter types) ─────────────────────────────────────
        if (widgetKind == "custom" && supportedCustom.contains(typeVal)
            && !comingSoon.contains(typeVal)) {

            QString fontFolder = obj["font"].toString();
            QString folderPath = projectDir + "/" + fontFolder;

            QMap<QString, QImage> strip = loadImageStrip(folderPath);

            WidgetEntry entry;
            entry.widgetType = "custom";
            entry.typeValue  = typeVal;
            entry.json       = obj;
            entry.fontFolder = fontFolder;
            entry.imageStrip = strip;

            // Ensure align is present in JSON (default "left")
            if (!entry.json.contains("align"))
                entry.json["align"] = "left";

            int idx = widgetList.size();
            widgetList.append(entry);
            renderCustomWidget(idx, true);

            // Register in font.json if not already listed
            bool found = false;
            for (const QJsonValue &fv : fontJsonItems)
                if (fv.toObject()["name"].toString() == fontFolder)
                { found = true; break; }
            if (!found && !fontFolder.isEmpty()) {
                QJsonObject fi;
                fi["name"]   = fontFolder;
                fi["bpp"]    = 16;
                fi["format"] = "png";
                fontJsonItems.append(fi);
            }
            continue;
        }

        // ── everything else: skip ─────────────────────────────────────────────
        skipped++;
    }

    // ── Finalise UI ───────────────────────────────────────────────────────────
    updateFontJsonDisplay();
    rebuildCurrentWidgetCombo();
    setProjectControlsEnabled(true);
    clearScrollArea();

    // ✨ Auto-select the first widget if any widgets were loaded
    if (!widgetList.isEmpty()) {
        ui->currentWidgetCombo_2->blockSignals(true);
        ui->currentWidgetCombo_2->setCurrentIndex(0);
        ui->currentWidgetCombo_2->blockSignals(false);

        selectWidget(0); // Populates scrollArea, spinboxes, and selection highlight
    } else {
        clearScrollArea();
    }

    QString msg = QString("Loaded %1 widget(s) from \"%2\"")
                      .arg(widgetList.size()).arg(QFileInfo(path).fileName());
    if (skipped > 0)
        msg += QString(" (%1 unsupported widget(s) skipped)").arg(skipped);
    statusBar()->showMessage(msg);
    ui->comboBox->setEnabled(false); // Added to prevent issues
}

// ─── Add anima widget ─────────────────────────────────────────────────────────
// Asks for a folder name, selects frame images, copies them to the project,
// then builds the anima JSON and renders frame 0 as a static preview.
bool MainWindow::addAnimaWidget()
{
    // ── 1. Ask for folder / animaicon name ───────────────────────────────────
    bool ok;
    QString folderName = QInputDialog::getText(
        this, "Anima Folder Name",
        "Enter the folder name for the animation frames\n"
        "(will be created inside the project directory):",
        QLineEdit::Normal, "anima", &ok);
    if (!ok || folderName.trimmed().isEmpty()) return false;
    folderName = folderName.trimmed();

    // ── 2. Select frame images ────────────────────────────────────────────────
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        QString("Select frame images for anima widget (folder: %1)").arg(folderName),
        "", "Images (*.png *.bmp)");
    if (files.isEmpty()) return false;

    // ── 3. Copy frames into project subfolder ─────────────────────────────────
    QString destFolder = projectDir + "/" + folderName;
    QDir().mkpath(destFolder);

    QMap<QString, QImage> strip;
    QString ext;
    int frameCount = 0;

    for (const QString &srcPath : files) {
        QFileInfo fi(srcPath);
        if (ext.isEmpty()) ext = fi.suffix().toLower();

        QString dst = destFolder + "/" + fi.fileName();
        if (QFile::exists(dst)) QFile::remove(dst);
        QFile::copy(srcPath, dst);

        QImage img(srcPath);
        if (!img.isNull()) {
            strip[fi.completeBaseName()] =
                img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            frameCount++;
        }
    }

    // ── 4. Measure frame 0 for auto w/h ──────────────────────────────────────
    QImage frame0;
    if (strip.contains("0"))       frame0 = strip["0"];
    else if (!strip.isEmpty())     frame0 = strip.first();

    int fw = frame0.isNull() ? 50 : frame0.width();
    int fh = frame0.isNull() ? 50 : frame0.height();

    // ── 5. Build JSON (exact key order per spec) ──────────────────────────────
    QJsonObject obj;
    obj["widget"]      = "custom";
    obj["type"]        = "anima";
    obj["x"]           = 0;
    obj["y"]           = 0;
    obj["w"]           = fw;
    obj["h"]           = fh;
    obj["time"]        = 1000;
    obj["turn"]        = 0;
    obj["animatype"]   = "normal";
    obj["animaicon"]   = folderName;
    obj["frame"]       = frameCount;
    obj["animabpp"]    = 16;
    obj["animaformat"] = ext.isEmpty() ? "png" : ext;

    // ── 6. Register in font.json ─────────────────────────────────────────────
    bool fontExists = false;
    for (int fi = 0; fi < fontJsonItems.size(); ++fi)
        if (fontJsonItems[fi].toObject()["name"].toString() == folderName)
        { fontExists = true; break; }
    if (!fontExists) {
        QJsonObject fontItem;
        fontItem["name"]   = folderName;
        fontItem["bpp"]    = 16;
        fontItem["format"] = ext.isEmpty() ? "png" : ext;
        fontJsonItems.append(fontItem);
        updateFontJsonDisplay();
    }

    // ── 7. Build entry and render ─────────────────────────────────────────────
    WidgetEntry entry;
    entry.widgetType = "custom";
    entry.typeValue  = "anima";
    entry.json       = obj;
    entry.fontFolder = folderName;
    entry.imageStrip = strip;

    int newIdx = widgetList.size();
    widgetList.append(entry);
    renderCustomWidget(newIdx);

    updateJsonDisplay();
    rebuildCurrentWidgetCombo();
    ui->currentWidgetCombo_2->blockSignals(true);
    ui->currentWidgetCombo_2->setCurrentIndex(newIdx);
    ui->currentWidgetCombo_2->blockSignals(false);
    selectWidget(newIdx);

    return true;
}

bool MainWindow::addRedpointWidget()
{
    // ── 1. Ask for folder name ───────────────────────────────────────────────────
    bool ok;
    QString folderName = QInputDialog::getText(
        this, "Redpoint Folder Name",
        "Enter the folder name for the redpoint images\n"
        "(will be created inside the project directory):",
        QLineEdit::Normal, "red_point", &ok);
    if (!ok || folderName.trimmed().isEmpty()) return false;
    folderName = folderName.trimmed();

    // ── 2. Select images ────────────────────────────────────────────────────────
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        QString("Select images for redpoint widget (folder: %1)").arg(folderName),
        "", "Images (*.png *.bmp)");
    if (files.isEmpty()) return false;

    // ── 3. Copy images into project subfolder ──────────────────────────────────
    QString destFolder = projectDir + "/" + folderName;
    QDir().mkpath(destFolder);

    QMap<QString, QImage> strip;
    QString ext;
    int imageCount = 0;

    for (const QString &srcPath : files) {
        QFileInfo fi(srcPath);
        if (ext.isEmpty()) ext = fi.suffix().toLower();

        QString dst = destFolder + "/" + fi.fileName();
        if (QFile::exists(dst)) QFile::remove(dst);
        QFile::copy(srcPath, dst);

        QImage img(srcPath);
        if (!img.isNull()) {
            strip[fi.completeBaseName()] =
                img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            imageCount++;
        }
    }

    // ── 4. Measure first image for auto w/h ────────────────────────────────────
    QImage firstImg;
    if (!strip.isEmpty()) firstImg = strip.first();

    int fw = firstImg.isNull() ? 20 : firstImg.width();
    int fh = firstImg.isNull() ? 20 : firstImg.height();

    // ── 5. Build JSON (CORRECT key order per spec) ─────────────────────────────
    QJsonObject obj;
    obj["widget"]   = "custom";
    obj["type"]     = "redpoint";
    obj["x"]        = 0;
    obj["y"]        = 0;
    obj["w"]        = fw;
    obj["h"]        = fh;
    obj["font"]     = folderName;
    obj["fontnum"]  = imageCount;

    // ── 6. Register in font.json ─────────────────────────────────────────────
    bool fontExists = false;
    for (int fi = 0; fi < fontJsonItems.size(); ++fi)
        if (fontJsonItems[fi].toObject()["name"].toString() == folderName)
        { fontExists = true; break; }
    if (!fontExists) {
        QJsonObject fontItem;
        fontItem["name"]   = folderName;
        fontItem["bpp"]    = 16;
        fontItem["format"] = ext.isEmpty() ? "png" : ext;
        fontJsonItems.append(fontItem);
        updateFontJsonDisplay();
    }

    // ── 7. Build entry and render ─────────────────────────────────────────────
    WidgetEntry entry;
    entry.widgetType = "custom";
    entry.typeValue  = "redpoint";
    entry.json       = obj;
    entry.fontFolder = folderName;
    entry.imageStrip = strip;

    int newIdx = widgetList.size();
    widgetList.append(entry);
    renderCustomWidget(newIdx);

    updateJsonDisplay();
    rebuildCurrentWidgetCombo();
    ui->currentWidgetCombo_2->blockSignals(true);
    ui->currentWidgetCombo_2->setCurrentIndex(newIdx);
    ui->currentWidgetCombo_2->blockSignals(false);
    selectWidget(newIdx);

    return true;
}

// ─── Add icon widget ──────────────────────────────────────────────────────────
// Asks the user to select a single image file (no folder dialog).
// Copies it to the project directory and builds the icon JSON.
bool MainWindow::addIconWidget()
{
    // ── 1. Select a single image ──────────────────────────────────────────────
    QString srcPath = QFileDialog::getOpenFileName(
        this, "Select Icon Image", "", "Images (*.png *.bmp)");
    if (srcPath.isEmpty()) return false;

    QFileInfo fi(srcPath);
    QString fileName = fi.fileName();

    // ── 2. Copy to project root ───────────────────────────────────────────────
    QString dst = projectDir + "/" + fileName;
    if (QFile::exists(dst)) QFile::remove(dst);
    QFile::copy(srcPath, dst);

    QImage img(srcPath);
    if (img.isNull()) {
        QMessageBox::critical(this, "Error", "Could not load image:\n" + srcPath);
        return false;
    }
    img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    // ── 3. Build JSON (exact key order per spec) ──────────────────────────────
    QJsonObject obj;
    obj["widget"]   = "custom";
    obj["type"]     = "icon";
    obj["x"]        = 0;
    obj["y"]        = 0;
    obj["w"]        = img.width();
    obj["h"]        = img.height();
    obj["bgcolor"]  = "0xFFFFFFFF";
    obj["bgrender"] = "0xFFFFFFFF";
    obj["bg"]       = fileName;

    // ── 4. Build entry and render ─────────────────────────────────────────────
    WidgetEntry entry;
    entry.widgetType = "custom";
    entry.typeValue  = "icon";
    entry.json       = obj;
    entry.fontFolder = fileName;
    entry.imageStrip["__icon__"] = img;

    int newIdx = widgetList.size();
    widgetList.append(entry);
    renderCustomWidget(newIdx);

    updateJsonDisplay();
    rebuildCurrentWidgetCombo();
    ui->currentWidgetCombo_2->blockSignals(true);
    ui->currentWidgetCombo_2->setCurrentIndex(newIdx);
    ui->currentWidgetCombo_2->blockSignals(false);
    selectWidget(newIdx);

    return true;
}

void MainWindow::on_pushButtonCreateIwfLz_clicked()
{
    DialogIwfLz dialog(this); // Modal dialog bound to App_Window as parent
    dialog.exec();             // Displays dialog modally
}