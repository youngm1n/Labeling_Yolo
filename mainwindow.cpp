#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMimeData>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QAbstractSlider>
#include <QScrollBar>

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
    imgEditor = ui->widgetImageViewer;

    // Class editor
    objClassEditor = new DialogObjectClassEditor(this);
    imgEditor->setClassEditor(objClassEditor);
    connect(objClassEditor, &DialogObjectClassEditor::updateClassInformation, this, &MainWindow::updateClassInformation);
    connect(objClassEditor, &DialogObjectClassEditor::updateClassInformation, imgEditor, &ImageViewer::updateClassInformation);

    // Init the image table
    auto headerLabel = QStringList() << "Name" << "Object Count" << "-";
    ui->tableWidgetImage->setColumnCount(headerLabel.count());
    ui->tableWidgetImage->setHorizontalHeaderLabels(headerLabel);
    ui->tableWidgetImage->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidgetImage->setSelectionMode(QTableWidget::SingleSelection);
    ui->tableWidgetImage->setSelectionBehavior(QTableWidget::SelectRows);
    connect(ui->tableWidgetImage, &QTableWidget::itemPressed, this, &MainWindow::pressedImageTableItem);
    connect(ui->tableWidgetImage, &QTableWidget::itemSelectionChanged, this, [this]() {
        pressedImageTableItem(ui->tableWidgetImage->selectedItems().first());
    });

    auto defaultHeight = ui->tableWidgetImage->verticalHeader()->defaultSectionSize() * 2;
    ui->tableWidgetImage->verticalHeader()->setDefaultSectionSize(defaultHeight);
    ui->tableWidgetImage->verticalHeader()->setIconSize(QSize(defaultHeight, defaultHeight));
    connect(ui->tableWidgetImage->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
        showImageThumbnailInTable();
    });

    // Init the object table
    headerLabel = QStringList() << "Object" << "X" << "Y" << "Width" << "Height" << "-";
    ui->tableWidgetLabel->setColumnCount(headerLabel.count());
    ui->tableWidgetLabel->setHorizontalHeaderLabels(headerLabel);
    ui->tableWidgetLabel->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidgetLabel->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgetLabel->setSelectionMode(QTableWidget::SingleSelection);
    ui->tableWidgetLabel->setSelectionBehavior(QTableWidget::SelectRows);
    connect(ui->tableWidgetLabel, &QTableWidget::itemPressed, this, &MainWindow::pressedLabelTableItem);
    connect(imgEditor, &ImageViewer::updateObjects, this, &MainWindow::updateObjects);

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
    showImageThumbnailInTable();
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
    objClassEditor->clear();
    selectedImgRow = -1;
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
        btn->setProperty("TABLE_ROW", i);
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
        // 1st column: thumbnail and name
        auto itemThumb = new QTableWidgetItem(baseName);
        ui->tableWidgetImage->setItem(row, TABLE_IMG_COL_NAME, itemThumb);
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

        // Open object class editor
        objClassEditor->exec();

        // Show thumbnail
        showImageThumbnailInTable();

        // Select the first image
        if (ui->tableWidgetImage->rowCount() > 0) {
            pressedImageTableItem(ui->tableWidgetImage->item(0, 0));
        }
    }

    // Increase table row
    timerImgLoad.setProperty("TABLE_ROW", ++row);
}

// Load object infomation from file
void MainWindow::loadObjectInfo(const QFileInfo &labelFileInfo, OBJECTS &objs)
{
    QFile labelFile(labelFileInfo.absoluteFilePath());
    if (labelFile.exists()) {
        if (labelFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&labelFile);
            QString line;
            QStringList data;
            int classNo = 0;
            QString className;
            QColor classColor;
            float x, y, w, h;
            while (stream.readLineInto(&line)) {
                data = line.split(" ");
                classNo = data.first().toInt();
                if (classNo < objClassNames.count()) {
                    className = objClassNames.at(classNo);
                    classColor = objClassColors.at(classNo);
                }
                x = data.at(1).toFloat();
                y = data.at(2).toFloat();
                w = data.at(3).toFloat();
                h = data.at(4).toFloat();
                objs.push_back(new object(classNo, className, classColor, x, y, w, h));

                // Insert classNo into class list, if it is new
                objClassEditor->insertNewClassNo(classNo);
            }
            labelFile.close();
        }
    }
    else {
        labelFile.open(QIODevice::WriteOnly);
        labelFile.close();
    }
}

void MainWindow::initObjectTable(OBJECTS &)
{

}

// Only load thumbnails displayed in the table
void MainWindow::showImageThumbnailInTable()
{
    auto rowStart = ui->tableWidgetImage->rowAt(0);
    auto rowEnd = ui->tableWidgetImage->rowAt(ui->tableWidgetImage->height());

    if (rowStart >= 0 && rowEnd >= 0) {
        for (int row = rowStart; row <= rowEnd; row++) {
            if (ui->tableWidgetImage->item(row, TABLE_IMG_COL_NAME)->data(Qt::DecorationRole).isNull()) {
                auto pix = QPixmap(imgFileInfos.at(row).absoluteFilePath()).scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                ui->tableWidgetImage->item(row, TABLE_IMG_COL_NAME)->setData(Qt::DecorationRole, pix);
            }
        }
    }
}

// Remove selected image
void MainWindow::pressedImageRemoveButton()
{
    auto row = sender()->property("TABLE_ROW").toInt();
}

// Remove selected object
void MainWindow::pressedObjectRemoveButton()
{

}

// Click image from image table
void MainWindow::pressedImageTableItem(QTableWidgetItem *item)
{
    selectedImgRow = item->row();

    auto imgFileInfo = imgFileInfos.at(selectedImgRow);
    auto labelFileInfo = labelFileInfos.at(selectedImgRow);

    // Load object infomation from label file and init object table with this
    OBJECTS objs;
    loadObjectInfo(labelFileInfo, objs);
    initObjectTable(objs);

    // Init label table
    auto labelRowCount = ui->tableWidgetLabel->rowCount();
    for (int row = 0; row < labelRowCount; row++) {
        ui->tableWidgetLabel->removeRow(0);
    }

    // Load each object and put into label table
    auto labelRow = 0;
    foreach (auto obj, objs) {
        insertNewObjIntoTable(obj, labelRow++);
    }

    // Display Image
    imgEditor->loadImage(imgFileInfo.absoluteFilePath(), objs);
}

// Click object from label table
void MainWindow::pressedLabelTableItem(QTableWidgetItem *item)
{
    auto row = item->row();
}

void MainWindow::changedObjectClass(int newClassNo)
{

}

void MainWindow::updateClassInformation(QStringList list, CLASS_COLORS colors)
{
    objClassNames = list;
    objClassColors = colors;
}

void MainWindow::updateObjects(OBJECTS objs)
{
    for (int objNo = 0; objNo < objs.count(); objNo++) {
        // Update table
        if (objNo < ui->tableWidgetLabel->rowCount()) {
            auto classCombo = static_cast<QComboBox *>(ui->tableWidgetLabel->cellWidget(objNo, TABLE_OBJ_COL_CLASS));
            classCombo->setCurrentIndex(objs.at(objNo)->getClassNo());
            ui->tableWidgetLabel->item(objNo, TABLE_OBJ_COL_X)->setText(QString().setNum(objs.at(objNo)->getYoloRect().x()));
            ui->tableWidgetLabel->item(objNo, TABLE_OBJ_COL_Y)->setText(QString().setNum(objs.at(objNo)->getYoloRect().y()));
            ui->tableWidgetLabel->item(objNo, TABLE_OBJ_COL_WIDTH)->setText(QString().setNum(objs.at(objNo)->getYoloRect().width()));
            ui->tableWidgetLabel->item(objNo, TABLE_OBJ_COL_HEIGHT)->setText(QString().setNum(objs.at(objNo)->getYoloRect().height()));
        }
        // Add new row into table
        else {
            insertNewObjIntoTable(objs.at(objNo), objNo);
        }
    }
}

void MainWindow::insertNewObjIntoTable(object *obj, int tableRow)
{
    ui->tableWidgetLabel->insertRow(tableRow);

    // 1st column: object's class (combobox)
    auto combo = new QComboBox;
    auto sizeComboIcon = combo->style()->pixelMetric(QStyle::PM_SmallIconSize);
    for (int i = 0; i < objClassNames.count(); i++) {
        combo->addItem(objClassNames.at(i));

        // Set color
        QPixmap pix(sizeComboIcon, sizeComboIcon);
        pix.fill(objClassColors.at(i));
        combo->setItemData(i, pix, Qt::DecorationRole);
    }
    combo->setCurrentIndex(obj->getClassNo());
    combo->setFocusPolicy(Qt::NoFocus);
    connect(combo, &QComboBox::currentIndexChanged, this, &MainWindow::changedObjectClass);
    ui->tableWidgetLabel->setCellWidget(tableRow, TABLE_OBJ_COL_CLASS, combo);

    // 2nd ~ 5th column (x, y, w, h)
    ui->tableWidgetLabel->setItem(tableRow, TABLE_OBJ_COL_X, new QTableWidgetItem(QString().setNum(obj->getYoloRect().center().x())));
    ui->tableWidgetLabel->setItem(tableRow, TABLE_OBJ_COL_Y, new QTableWidgetItem(QString().setNum(obj->getYoloRect().center().y())));
    ui->tableWidgetLabel->setItem(tableRow, TABLE_OBJ_COL_WIDTH, new QTableWidgetItem(QString().setNum(obj->getYoloRect().width())));
    ui->tableWidgetLabel->setItem(tableRow, TABLE_OBJ_COL_HEIGHT, new QTableWidgetItem(QString().setNum(obj->getYoloRect().height())));
    // Set text alignment
    for (int col = TABLE_OBJ_COL_X; col <= TABLE_OBJ_COL_HEIGHT; col++) {
        ui->tableWidgetLabel->item(tableRow, col)->setTextAlignment(Qt::AlignCenter);
    }

    // 6th column: remove
    auto btnRemove = new QPushButton("Remove");
    btnRemove->setProperty("TABLE_ROW", tableRow);
    connect(btnRemove, &QPushButton::pressed, this, &MainWindow::pressedObjectRemoveButton);
    ui->tableWidgetLabel->setCellWidget(tableRow, TABLE_OBJ_COL_REMOVE, btnRemove);
}
