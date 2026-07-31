#include "bkground_helper.h"
#include "qgraphicsitem.h"
#include "ui_bkground_helper.h"
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QDebug>  // For debugging
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QCloseEvent>
#include <QTimeEdit>
#include <QPalette>
#include <QColor>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>

bkground_helper::bkground_helper(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::bkground_helper)
{
    ui->setupUi(this);

    // Connect Buttons and functions
    connect(ui->pushButton_2, &QPushButton::clicked, this, &bkground_helper::upload_image);

    // Create the graphics scene
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    // Get the exact dimensions of the graphics view from the UI
    int viewWidth = ui->graphicsView->width();
    int viewHeight = ui->graphicsView->height();

    //qDebug() << "Graphics View dimensions:" << viewWidth << "x" << viewHeight;

    // Set scene to match the view dimensions exactly
    scene->setSceneRect(0, 0, viewWidth, viewHeight);

    // Set black background directly on the scene, not via stylesheet
    scene->setBackgroundBrush(Qt::black);

    // Remove scrollbars
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Disable any view transformations that might affect alignment
    ui->graphicsView->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->graphicsView->resetTransform();

    // Create rounded rectangle that fills the entire scene
    int penWidth = 4; // Thickness of the green line

    // Create path that goes right to the edges
    QPainterPath roundedRect;

    // Calculate radius for 70% roundness
    // Using the smaller dimension as reference
    int smallerDimension = qMin(viewWidth, viewHeight);
    int radius = (smallerDimension * 50) / 100 / 2; // 50% of half the smaller dimension

    // Account for pen width - draw half inside, half outside
    int offset = penWidth / 2;

    // Create rectangle that goes to the very edges
    roundedRect.addRoundedRect(offset, offset,
                               viewWidth - 2*offset, viewHeight - 2*offset,
                               radius, radius);

    // Add the rounded rectangle to the scene
    QGraphicsPathItem* roundedRectItem = new QGraphicsPathItem();
    roundedRectItem->setPath(roundedRect);

    QPen greenPen(QColor(0, 255, 0), penWidth); // Bright green
    greenPen.setCosmetic(false); // NOT cosmetic - will scale with view
    greenPen.setJoinStyle(Qt::MiterJoin);
    greenPen.setCapStyle(Qt::SquareCap);
    roundedRectItem->setPen(greenPen);
    roundedRectItem->setBrush(QBrush(Qt::NoBrush));
    roundedRectItem->setZValue(1000);

    scene->addItem(roundedRectItem);

    // Option 2: Alternative approach - draw directly on the viewport
    // This ensures the lines are exactly at the view edges
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

bkground_helper::~bkground_helper()
{
    delete scene; // Don't forget to delete the scene
    delete ui;
}

void bkground_helper::upload_image()
{
    // Open file dialog to select an image
    QString file_filter = "PNG Files (*.png);;BMP Files (*.bmp)";
    QString file_path = QFileDialog::getOpenFileName(
        this,
        "Upload Image",
        "",
        file_filter
        );

    if (file_path.isEmpty()) {  // User cancelled the dialog
        return;
    }

    // Check file extension
    if (!file_path.endsWith(".png", Qt::CaseInsensitive) &&
        !file_path.endsWith(".bmp", Qt::CaseInsensitive)) {
        QMessageBox::critical(
            this,
            "Bad File Type",
            "Select a valid image that's at least PNG or BMP"
            );
        return;
    }

    // Load the image
    QImage image(file_path);

    // Check image dimensions
    const int max_width = 320;
    const int max_height = 385;

    if (image.width() > max_width || image.height() > max_height) {
        QMessageBox::critical(
            this,
            "Image Too Large",
            "Bad File"
            );
        return;
    }

    // Create QPixmap from QImage
    QPixmap pixmap = QPixmap::fromImage(image);

    // Scale the pixmap
    QPixmap scaled_pixmap = pixmap.scaled(
        max_width,
        max_height,
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation
        );

    // Create pixmap item and add it to the scene
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(scaled_pixmap);

    // Position the image
    int viewWidth = ui->graphicsView->width();
    int viewHeight = ui->graphicsView->height();

    // Center the image in the view
    int xPos = (viewWidth - max_width) / 2;
    int yPos = (viewHeight - max_height) / 2;
    pixmapItem->setPos(xPos, yPos);

    // Add the pixmap item to the scene
    scene->addItem(pixmapItem);

    // Make sure the image is behind the green border
    pixmapItem->setZValue(-1);  // Put behind other items
}
