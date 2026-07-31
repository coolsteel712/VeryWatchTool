#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "make_watch_face.h"
#include "bitmap_font.h"
#include "debug.h"

#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QTextCursor>
#include <regex>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupCustomUi();

    connect(ui->pushButtonSelectFolder, &QPushButton::clicked, this, &MainWindow::onSelectFolderClicked);
    connect(ui->pushButtonBuildIwf, &QPushButton::clicked, this, &MainWindow::onBuildIwfClicked);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupCustomUi()
{
    // Inject Format Selection drop-down above the action buttons
    QHBoxLayout *formatLayout = new QHBoxLayout();
    m_labelFormat = new QLabel("Select Target Pixel Format:", this);
    m_comboBoxFormat = new QComboBox(this);

    // Populate binary formats supported by bitmap_font.h
    m_comboBoxFormat->addItem("RGB565 (Default)", 0x85);      // 0x80 | 0x05
    m_comboBoxFormat->addItem("ARGB565 (Alpha 16-bit)", 0x86); // 0x80 | 0x06
    m_comboBoxFormat->addItem("BGR565 (Swap Color)", 0x87);   // 0x80 | 0x07
    m_comboBoxFormat->addItem("ABGR565 (Alpha Swap)", FONT_FORMAT_ABGR565);
    m_comboBoxFormat->addItem("RGB222 (6-bit)", FONT_FORMAT_RGB222);
    m_comboBoxFormat->addItem("ARGB888 (32-bit TrueColor)", FONT_FORMAT_ARGB888);
    m_comboBoxFormat->addItem("MONO4 (Grayscale 4-bit)", FONT_FORMAT_MONO4);

    formatLayout->addWidget(m_labelFormat);
    formatLayout->addWidget(m_comboBoxFormat, 1);

    // Insert control into central layout before line separator
    QVBoxLayout *centralLayout = qobject_cast<QVBoxLayout *>(ui->centralwidget->layout());
    if (centralLayout) {
        centralLayout->insertLayout(2, formatLayout);
    }
}

void MainWindow::appendLog(const QString &text)
{
    QString timestamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ");
    ui->plainTextEdit->appendPlainText(timestamp + text);
    ui->plainTextEdit->moveCursor(QTextCursor::End);
}

uint8_t MainWindow::getSelectedFormatByte() const
{
    return static_cast<uint8_t>(m_comboBoxFormat->currentData().toInt());
}

bool MainWindow::validateFolderStructure(const QString &folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        appendLog("Error: Selected directory does not exist.");
        return false;
    }

    // Main descriptor file check
    if (!dir.exists("iwf.json")) {
        appendLog("Error: Mandatory descriptor file 'iwf.json' was not found in: " + folderPath);
        return false;
    }

    return true;
}

void MainWindow::scanAndDetectFolder(const QString &folderPath)
{
    appendLog("Scanning folder assets: " + folderPath);

    QDir dir(folderPath);
    if (dir.exists("iwf1.json")) {
        appendLog("Detected secondary descriptor: iwf1.json");
    }
    if (dir.exists("font.json")) {
        appendLog("Detected font layout manifest: font.json");
    }

    // Inspect files and auto-detect suffix variations like make_watch_face.cpp
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    std::regex suffix24Bit("(.*)(_24bit\\.(bmp|BMP|png|PNG))");
    std::regex suffixLz("(.*)(_lz\\.(bmp|BMP|png|PNG))");
    std::regex suffix565("(.*)(_565\\.bmp)");

    int count24bit = 0;
    int countLz = 0;
    int count565 = 0;

    for (const QFileInfo &info : fileList) {
        std::string fileName = info.fileName().toStdString();
        if (std::regex_match(fileName, suffix24Bit)) {
            count24bit++;
        } else if (std::regex_match(fileName, suffixLz)) {
            countLz++;
        } else if (std::regex_match(fileName, suffix565)) {
            count565++;
        }
    }

    appendLog(QString("Auto-detection summary: %1 24bit assets, %2 LZ compressed assets, %3 pre-rendered 565 assets.")
                  .arg(count24bit).arg(countLz).arg(count565));

    // Recommend output formats dynamically based on asset inspection
    if (count24bit > 0) {
        appendLog("Auto-detection hint: _24bit suffix detected. Recommending RGB565 / ARGB565 target format.");
    }
}

void MainWindow::onSelectFolderClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Watch Face Asset Directory"),
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) {
        return;
    }

    // Ensure trailing slash formatting for compatibility with C-style path concatenation
    if (!dir.endsWith('/') && !dir.endsWith('\\')) {
#ifdef Q_OS_WIN
        dir += '\\';
#else
        dir += '/';
#endif
    }

    if (!validateFolderStructure(dir)) {
        QMessageBox::critical(this, tr("Validation Error"), tr("The selected folder does not contain a valid 'iwf.json' file."));
        return;
    }

    m_selectedFolderPath = dir;
    appendLog("Selected asset path set to: " + m_selectedFolderPath);

    // Auto-detect asset formats inside directory
    scanAndDetectFolder(m_selectedFolderPath);
}

void MainWindow::onBuildIwfClicked()
{
    if (m_selectedFolderPath.isEmpty()) {
        QMessageBox::warning(this, tr("Execution Error"), tr("Please select a valid watch face folder first."));
        return;
    }

    if (!validateFolderStructure(m_selectedFolderPath)) {
        QMessageBox::critical(this, tr("Build Error"), tr("Invalid folder context. Missing iwf.json."));
        return;
    }

    QString saveFilePath = QFileDialog::getSaveFileName(this, tr("Export Watch Face File"),
                                                        "watchface.iwf",
                                                        tr("IWF Binary Watchface (*.iwf);;All Files (*)"));
    if (saveFilePath.isEmpty()) {
        return;
    }

    uint8_t selectedFormat = getSelectedFormatByte();
    appendLog(QString("Initiating .iwf compilation | Target Format Code: 0x%1").arg(selectedFormat, 2, 16, QChar('0')));

    // Instantiating mkWatchFace generator engine
    mkWatchFace builder;

    std::string srcPath = m_selectedFolderPath.toStdString();
    std::string destPath = saveFilePath.toStdString();

    // Compile into output container
    uint32_t resultSize = builder.makeIwfFile(srcPath, destPath, selectedFormat);

    if (resultSize == 1 || resultSize == 0) {
        appendLog("Error: Failed to compile watch face .iwf container.");
        QMessageBox::critical(this, tr("Build Failed"), tr("Compilation encountered an error. Check console logs."));
    } else {
        appendLog(QString("Build succeeded. Successfully generated '%1' (Size: %2 bytes).")
                      .arg(saveFilePath)
                      .arg(resultSize));
        QMessageBox::information(this, tr("Success"), tr("IWF Watchface compiled successfully!\nSize: %1 bytes").arg(resultSize));
    }
}