#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMimeData>
#include <QDragEnterEvent>
#include <QFileDialog>

enum ENUM_TABLE_IMG_COL { TABLE_IMG_COL_NAME, TABLE_IMG_COL_COUNT, TABLE_IMG_COL_REMOVE, TABLE_IMG_COL_TOTAL };
enum ENUM_TABLE_OBJ_COL { TABLE_OBJ_COL_CLASS,
                          TABLE_OBJ_COL_X, TABLE_OBJ_COL_Y, TABLE_OBJ_COL_WIDTH, TABLE_OBJ_COL_HEIGHT,
                          TABLE_OBJ_COL_REMOVE,
                          TABLE_OBJ_COL_TOTAL };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Accept drag and drop on window
    setAcceptDrops(true);

    // Hide Image table
    ui->widgetImageTable->hide();

    // Set image view widget
    imgView = ui->widgetImageViewer;

    // Init the image table
    auto headerLabel = QStringList() << "Name" << "Object Count" << "-";
    ui->tableWidgetImage->setColumnCount(headerLabel.count());
    ui->tableWidgetImage->setHorizontalHeaderLabels(headerLabel);
    ui->tableWidgetImage->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidgetImage->setSelectionMode(QTableWidget::SingleSelection);
    ui->tableWidgetImage->setSelectionBehavior(QTableWidget::SelectRows);
    connect(ui->tableWidgetImage, &QTableWidget::itemPressed, this, &MainWindow::pressedImageTableItem);

    // Init the object table
    headerLabel = QStringList() << "Object" << "X" << "Y" << "Width" << "Height" << "-";
    ui->tableWidgetLabel->setColumnCount(headerLabel.count());
    ui->tableWidgetLabel->setHorizontalHeaderLabels(headerLabel);
    ui->tableWidgetLabel->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidgetLabel->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgetLabel->setSelectionMode(QTableWidget::SingleSelection);
    ui->tableWidgetLabel->setSelectionBehavior(QTableWidget::SelectRows);
    connect(ui->tableWidgetLabel, &QTableWidget::itemPressed, this, &MainWindow::pressedLabelTableItem);

    // Connect image file load timer
    connect(&timerImgLoad, &QTimer::timeout, this, &MainWindow::timoutLoadImageFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{

}

void MainWindow::resizeEvent(QResizeEvent *event)
{

}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept drag and drop for only "images" folder
    if (event->mimeData()->urls().count() == 1) {
        QFileInfo info(event->mimeData()->urls().first().toLocalFile());
        if (info.isDir()) {
            event->acceptProposedAction();
        }
    }
}

// Select image folder, when the image folder is dropped
void MainWindow::dropEvent(QDropEvent *event)
{
    loadImageFolder(QDir(event->mimeData()->urls().first().toLocalFile()));
}

// Select image folder by dialog
void MainWindow::on_toolButtonLoadImgFolder_clicked()
{
    auto dir = QFileDialog::getExistingDirectory(this, tr("Open image folder"), "./", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    loadImageFolder(dir);
}

// Load image folder
void MainWindow::loadImageFolder(QDir dirImage)
{
    // Init parameters
    btnRemoveImg.clear();
    imgFileInfos.clear();
    labelFileInfos.clear();
    classList.clear();
    ui->progressBarImgLoad->setValue(0);

    // Set image folder
    ui->lineEditImageFolderPath->setText(dirImage.absolutePath());

    // Find label folder
    auto dirLabel = dirImage;
    dirLabel.cdUp();
    if (!dirLabel.cd("labels")) {
        // Make 'labels' directory, when there is no
        dirLabel.mkdir(dirLabel.absolutePath() + "/labels");
        dirLabel.cd("labels");
    }

    // Set label folder
    ui->lineEditLabelFolderPath->setText(dirLabel.absolutePath());

    // get image file infomations (only jpg)
    imgFileInfos = dirImage.entryInfoList(QStringList() << "*.jpg" << "*.jpeg" << "*.JPG" << "*.JPEG", QDir::Files);
    labelFileInfos.reserve(imgFileInfos.count());
    btnRemoveImg.reserve(imgFileInfos.count());

    // Hide Drag and Drop desc. and show image table
    ui->frameDragAndDrop->hide();
    ui->widgetImageTable->show();

    // Init image table
    // Reset img table's column resize mode -> to increase table insert speed
    for (int col = 0; col < TABLE_IMG_COL_TOTAL; col++) {
        ui->tableWidgetImage->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Interactive);
    }
    // Init image table's row and set remove button in advance -> to increase speed
    ui->tableWidgetImage->setRowCount(imgFileInfos.count());
    for (int i = 0; i < imgFileInfos.count(); i++) {
        auto btn = new QPushButton("Remove");
        ui->tableWidgetImage->setCellWidget(i, TABLE_IMG_COL_REMOVE, btn);
        connect(btn, &QPushButton::pressed, this, &MainWindow::pressedImageRemoveButton);
        btnRemoveImg.push_back(btn);
    }

    // Start timer for loading jpg images -> to prevent freezing during load a log of image
    timerImgLoad.setProperty("TABLE_ROW", 0);
    timerImgLoad.start(0);
}

// Load jpg file and put it into the image table when timeout
void MainWindow::timoutLoadImageFile()
{
    // Get current table row
    auto row = timerImgLoad.property("TABLE_ROW").toInt();

    if (row < imgFileInfos.count()) {
        // Get image file info
        auto imgFileInfo = imgFileInfos.at(row);
        // Get base name -> QFileInfo's baseName() ocurred some mulfunction....
        auto baseName = imgFileInfo.fileName().left(imgFileInfo.fileName().size() - imgFileInfo.fileName().split(".").last().size() - 1);
        // Get label file info
        auto labelFileInfo = QFileInfo(ui->lineEditLabelFolderPath->text() + "/" + baseName + ".txt");
        labelFileInfos.push_back(labelFileInfo);

        // Set table items
        // 1st column: name
        ui->tableWidgetImage->setItem(row, TABLE_IMG_COL_NAME, new QTableWidgetItem(baseName));
        // 2nd column: object count
        OBJECTS objs;
        loadObjectInfo(labelFileInfo, objs);
        ui->tableWidgetImage->setItem(row, TABLE_IMG_COL_COUNT, new QTableWidgetItem(QString().setNum(objs.count())));
        ui->tableWidgetImage->item(row, TABLE_IMG_COL_COUNT)->setTextAlignment(Qt::AlignCenter);

        // Update loading progress bar
        ui->progressBarImgLoad->setValue((row + 1) / static_cast<float>(imgFileInfos.count()) * 100.0f + 0.5f);
    }
    else {
        // Finish image loading
        timerImgLoad.stop();
        ui->progressBarImgLoad->hide();

        // Set resize mode for image table column
        ui->tableWidgetImage->horizontalHeader()->setSectionResizeMode(TABLE_IMG_COL_NAME, QHeaderView::Stretch);
        ui->tableWidgetImage->horizontalHeader()->setSectionResizeMode(TABLE_IMG_COL_COUNT, QHeaderView::ResizeToContents);
        ui->tableWidgetImage->horizontalHeader()->setSectionResizeMode(TABLE_IMG_COL_REMOVE, QHeaderView::ResizeToContents);
    }

    // Increase table row
    timerImgLoad.setProperty("TABLE_ROW", ++row);
}

// Load object infomation from file
void MainWindow::loadObjectInfo(const QFileInfo &labelFileInfo, OBJECTS &objs)
{
    QFile labelFile(labelFileInfo.absoluteFilePath());
    if (labelFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&labelFile);
        QString line;
        QStringList data;
        int classNo = 0;
        float x, y, w, h;
        while (stream.readLineInto(&line)) {
            data = line.split(" ");
            classNo = data.first().toInt();
            x = data.at(1).toFloat();
            y = data.at(2).toFloat();
            w = data.at(3).toFloat();
            h = data.at(4).toFloat();
            objs.push_back(new object(classNo, x, y, w, h));

            // Insert classNo into class list, if it is new
            if (classNo > classList.count() - 1) {
                classList.push_back(QString("class_%1").arg(classNo));
            }
        }
        labelFile.close();
    }
}

void MainWindow::initObjectTable(OBJECTS &)
{

}

// Remove selected image
void MainWindow::pressedImageRemoveButton()
{

}

// Click image from image table
void MainWindow::pressedImageTableItem(QTableWidgetItem *item)
{
    auto row = item->row();

    auto imgFileInfo = imgFileInfos.at(row);
    auto labelFileInfo = labelFileInfos.at(row);

    // Load object infomation from label file and init object table with this
    OBJECTS objs;
    loadObjectInfo(labelFileInfo, objs);
    initObjectTable(objs);
}

// Click object from label table
void MainWindow::pressedLabelTableItem(QTableWidgetItem *item)
{
    auto row = item->row();
}
