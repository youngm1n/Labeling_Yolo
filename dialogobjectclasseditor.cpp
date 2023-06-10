#include "dialogobjectclasseditor.h"
#include "ui_dialogobjectclasseditor.h"

#include <QRandomGenerator>
#include <QColorDialog>
#include <QFile>
#include <QTextStream>

#define OBJECT_LIST_FILE    "ObjectList.txt"

enum ENUM_TABLE_COL { TABLE_COL_NO, TABLE_COL_COLOR, TABLE_COL_CLASS, TABLE_COL_TOTAL };

DialogObjectClassEditor::DialogObjectClassEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogObjectClassEditor)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    connect(ui->pushButton, &QPushButton::pressed, this, &DialogObjectClassEditor::close);
}

DialogObjectClassEditor::~DialogObjectClassEditor()
{
    delete ui;
}

void DialogObjectClassEditor::initTable()
{
    lastAddedClassNo = 0;
    ui->tableWidget->clearContents();

    // Load list file
    QFile file(OBJECT_LIST_FILE);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        objClassList = stream.readAll().split("\n", Qt::SkipEmptyParts);
        file.close();
    }
    if (objClassSet.count() > objClassList.count()) {
        for (int i = objClassList.count(); i < objClassSet.count(); i++) {
            objClassList.push_back(QString("object_%1").arg(i));
        }
    }
    auto countClass = objClassList.isEmpty() ? objClassSet.count() : objClassList.count();

    // Init table
    disconnect(ui->tableWidget, &QTableWidget::itemChanged, this, &DialogObjectClassEditor::objectClassNameChanged);
    ui->tableWidget->setColumnCount(TABLE_COL_TOTAL);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "No" << "Color" << "Class name");
    ui->tableWidget->setRowCount(countClass);

    // Insert item
    for (int row = 0; row < countClass; row++) {
        ui->tableWidget->setItem(row, TABLE_COL_NO, new QTableWidgetItem(QString().setNum(row)));
        ui->tableWidget->item(row, TABLE_COL_NO)->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setCellWidget(row, TABLE_COL_COLOR, getColorButton(row));
        ui->tableWidget->setItem(row, TABLE_COL_CLASS, new QTableWidgetItem(objClassList.at(row)));
    }

    // Insert new class maker
    ui->tableWidget->insertRow(countClass);
    ui->tableWidget->setItem(countClass, TABLE_COL_NO, new QTableWidgetItem(" "));
    ui->tableWidget->setItem(countClass, TABLE_COL_CLASS, new QTableWidgetItem("Add new class..."));
    connect(ui->tableWidget, &QTableWidget::itemChanged, this, &DialogObjectClassEditor::objectClassNameChanged);
}

int DialogObjectClassEditor::exec()
{
    initTable();
    return QDialog::exec();
}

void DialogObjectClassEditor::clear()
{
    objClassSet.clear();
    objClassList.clear();
    objClassColors.clear();
    disconnect(ui->tableWidget, &QTableWidget::itemChanged, this, &DialogObjectClassEditor::objectClassNameChanged);
    ui->tableWidget->clearContents();
}

void DialogObjectClassEditor::insertNewClassNo(const int &no)
{
    if (objClassSet.find(no) == objClassSet.end()) {
        objClassSet.insert(no);
    }
}

int DialogObjectClassEditor::getLastAddedClassNo()
{
    return lastAddedClassNo;
}

QPushButton *DialogObjectClassEditor::getColorButton(int row)
{
    QColor color;
    switch (row) {
    case 0:     color = QColor(Qt::green);  break;
    case 1:     color = QColor(Qt::blue);   break;
    case 2:     color = QColor(Qt::cyan);   break;
    case 3:     color = QColor(Qt::yellow); break;
    default:    color = QColor(QRandomGenerator::global()->bounded(255),
                       QRandomGenerator::global()->bounded(255),
                       QRandomGenerator::global()->bounded(255));
    }

    return getColorButton(row, color);
}

QPushButton *DialogObjectClassEditor::getColorButton(int row, QColor color)
{
    auto btnColor = new QPushButton("");
    btnColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
    btnColor->setProperty("COLOR", color);

    auto dlgColor = new QColorDialog(color);
    dlgColor->setProperty("ROW", row);
    connect(btnColor, &QPushButton::clicked, dlgColor, &QColorDialog::show);
    connect(dlgColor, &QColorDialog::colorSelected, this, &DialogObjectClassEditor::classColorChanged);

    return btnColor;
}

void DialogObjectClassEditor::objectClassNameChanged(QTableWidgetItem *item)
{
    if (item->column() == 0) {
        item->setText(item->row() != ui->tableWidget->rowCount() -1 ? QString().setNum(item->row()) : "");
    }
    else {
        if (item->row() == ui->tableWidget->rowCount() - 1 && item->column() == TABLE_COL_CLASS) {
            if (item->text().isEmpty()) {
                item->setText("Add new class...");
            }
            else {
                if (item->text() != "Add new class...") {
                    ui->tableWidget->insertRow(item->row() + 1);
                    ui->tableWidget->setItem(item->row() + 1, TABLE_COL_CLASS, new QTableWidgetItem("Add new class..."));

                    ui->tableWidget->setItem(item->row(), TABLE_COL_NO, new QTableWidgetItem(" "));
                    ui->tableWidget->item(item->row(), TABLE_COL_NO)->setTextAlignment(Qt::AlignCenter);
                    ui->tableWidget->setCellWidget(item->row(), TABLE_COL_COLOR, getColorButton(item->row()));
                    ui->tableWidget->item(item->row(), TABLE_COL_NO)->setText(QString().setNum(item->row()));
                    lastAddedClassNo = item->row();
                }
            }
        }
    }
}

void DialogObjectClassEditor::classColorChanged(const QColor &color)
{
    auto row = sender()->property("ROW").toInt();
    auto btnColor = reinterpret_cast<QPushButton *>(ui->tableWidget->cellWidget(row, TABLE_COL_COLOR));
    btnColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));
}

void DialogObjectClassEditor::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    objClassList.clear();
    objClassColors.clear();
    for (int row = 0; row < ui->tableWidget->rowCount() - 1; row++) {
        objClassList.push_back(ui->tableWidget->item(row, TABLE_COL_CLASS)->text());
        objClassColors.push_back(ui->tableWidget->cellWidget(row, TABLE_COL_COLOR)->property("COLOR").value<QColor>());
    }

    emit updateClassInformation(objClassList, objClassColors);

    // Save list file
    QFile file(OBJECT_LIST_FILE);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        for (int i = 0; i < ui->tableWidget->rowCount() - 1; i++) {
            stream << ui->tableWidget->item(i, TABLE_COL_CLASS)->text() << "\n";
        }
        file.close();
    }
}

