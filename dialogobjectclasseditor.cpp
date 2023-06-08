#include "dialogobjectclasseditor.h"
#include "ui_dialogobjectclasseditor.h"

#include <QRandomGenerator>
#include <QColorDialog>

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

void DialogObjectClassEditor::clear()
{
    objClassSet.clear();
    disconnect(ui->tableWidget, &QTableWidget::itemChanged, this, &DialogObjectClassEditor::objectClassNameChanged);
    ui->tableWidget->clearContents();
}

void DialogObjectClassEditor::insertNewClassNo(const int &no)
{
    if (objClassSet.find(no) == objClassSet.end()) {
        objClassSet.insert(no);
    }
}

void DialogObjectClassEditor::getClassInformation(QStringList &classList, CLASS_COLORS &classColors)
{
    if (ui->tableWidget->rowCount() > 1) {
        for (int row = 0; row < ui->tableWidget->rowCount() - 1; row++) {
            classList.push_back(ui->tableWidget->item(row, TABLE_COL_CLASS)->text());
            classColors.push_back(*(QColor *)(ui->tableWidget->cellWidget(row, TABLE_COL_COLOR)->property("COLOR").toLongLong()));
        }
    }
}

const QStringList DialogObjectClassEditor::getClassList()
{
    QStringList list;

    if (ui->tableWidget->rowCount() > 1) {
        for (int row = 0; row < ui->tableWidget->rowCount() - 1; row++) {
            list.push_back(ui->tableWidget->item(row, TABLE_COL_CLASS)->text());
        }
    }

    return list;
}

const CLASS_COLORS DialogObjectClassEditor::getClassColors()
{
    CLASS_COLORS colors;

    if (ui->tableWidget->rowCount() > 1) {
        for (int row = 0; row < ui->tableWidget->rowCount() - 1; row++) {
            colors.push_back(*(QColor *)(ui->tableWidget->cellWidget(row, TABLE_COL_COLOR)->property("COLOR").toLongLong()));
        }
    }

    return colors;
}

QPushButton *DialogObjectClassEditor::getColorButton(int row)
{
    auto btnColor = new QPushButton("");
    QColor *color;
    switch (row) {
    case 0:     color = new QColor(Qt::green);  break;
    case 1:     color = new QColor(Qt::blue);   break;
    case 2:     color = new QColor(Qt::cyan);   break;
    case 3:     color = new QColor(Qt::yellow); break;
    default:    color = new QColor(QRandomGenerator::global()->bounded(255), QRandomGenerator::global()->bounded(255), QRandomGenerator::global()->bounded(255));
    }
    btnColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color->red()).arg(color->green()).arg(color->blue()));
    btnColor->setProperty("COLOR", (quint64)color);

    auto dlgColor = new QColorDialog(*color);
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

    // Save new color into table
    *reinterpret_cast<QColor *>(btnColor->property("COLOR").toULongLong()) = color;
}

int DialogObjectClassEditor::exec()
{
    auto count = objClassSet.count();

    // Init table
    ui->tableWidget->setColumnCount(TABLE_COL_TOTAL);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "No" << "Color" << "Class name");
    ui->tableWidget->setRowCount(count);
    for (int row = 0; row < count; row++) {
        ui->tableWidget->setItem(row, TABLE_COL_NO, new QTableWidgetItem(QString().setNum(row)));
        ui->tableWidget->item(row, TABLE_COL_NO)->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setCellWidget(row, TABLE_COL_COLOR, getColorButton(row));
        ui->tableWidget->setItem(row, TABLE_COL_CLASS, new QTableWidgetItem(QString("class_%1").arg(row)));
    }
    ui->tableWidget->insertRow(count);
    ui->tableWidget->setItem(count, TABLE_COL_NO, new QTableWidgetItem(" "));
    ui->tableWidget->setItem(count, TABLE_COL_CLASS, new QTableWidgetItem("Add new class..."));
    connect(ui->tableWidget, &QTableWidget::itemChanged, this, &DialogObjectClassEditor::objectClassNameChanged);

    return QDialog::exec();
}
