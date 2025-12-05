#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QColorDialog>
#include <QPalette>
#include <QDebug>
#include <QGridLayout>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <windows.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    QMenu *fileMenu = menuBar()->addMenu("File");
    QMenu *setBackgroundMenu = menuBar()->addMenu("SetBckgColor");
    QMenu *fromfile = menuBar()->addMenu("set_from_file");
    QAction *saveAction = new QAction("Save Colors", this);
    QAction *setBackgroundAction = new QAction("Wybierz kolor tła", this);
    QAction *setfileF = new QAction("Select_file", this);
    fileMenu->addAction(saveAction);
    setBackgroundMenu->addAction(setBackgroundAction);
    fromfile->addAction(setfileF);
    connect(saveAction, &QAction::triggered, this, &MainWindow::on_saveToFile_clicked);
    connect(setBackgroundAction, &QAction::triggered, this, &MainWindow::on_setBackgroundColor_clicked);
    connect(setfileF, &QAction::triggered, this, &MainWindow::on_set_from_file_clicked);
    colorArray.resize(16, std::vector<std::vector<int>>(16, std::vector<int>(3, 0)));

    QGridLayout *layout = new QGridLayout(ui->centralwidget);
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            int index = i * 16 + j;
            QPushButton *colorButton = new QPushButton(this);
            colorButton->setFixedSize(40, 40);
            colorButton->setStyleSheet("background-color: grey;");
            layout->addWidget(colorButton, i, j);

            connect(colorButton, &QPushButton::clicked, this, [this, index]() {
                on_colorButton_clicked(index);
            });
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{


    QColor color = QColorDialog::getColor(selectedColor, this, "Wybierz kolor");

    if (color.isValid()) {
        selectedColor = color;


        QPalette palette = ui->pushButton->palette();
        palette.setColor(QPalette::Button, selectedColor);
        ui->pushButton->setPalette(palette);
        ui->pushButton->setAutoFillBackground(true);


        qDebug() << "Wybrany kolor RGB:" << selectedColor.red() << selectedColor.green() << selectedColor.blue();

    }
}

void MainWindow::on_colorButton_clicked(int index)
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Wybierz kolor");
    HANDLE hSerial = CreateFileA("COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć portu COM5");
        return;
    }

    if (color.isValid()) {
        colorArray[index / 16][index % 16][0] = color.red();
        colorArray[index / 16][index % 16][1] = color.green();
        colorArray[index / 16][index % 16][2] = color.blue();

        QPushButton *button = static_cast<QPushButton*>(sender());
        if (button) {
            button->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
                                      .arg(color.red())
                                      .arg(color.green())
                                      .arg(color.blue()));

            qDebug() << "Przycisk " << index << " - Wybrany kolor RGB:"
                     << color.red() << color.green() << color.blue();
        }

        char buffer[32];
        sprintf(buffer, "%d,%d,%d,%d\n", index, color.red(), color.green(), color.blue());
        qDebug() << "Wysyłam: " << buffer;

        DWORD bytesWritten;
        WriteFile(hSerial, buffer, strlen(buffer), &bytesWritten, NULL);
        CloseHandle(hSerial);
    }
}

void MainWindow::on_saveToFile_clicked()
{
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString filePath = appDirPath + "/colors.txt";

    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file for writing: " + file.errorString());
        return;
    }

    QTextStream out(&file);

    for (const auto& row : colorArray) {
        for (const auto& color : row) {
            out << color[0] << " " << color[1] << " " << color[2] << "\t"; // RGB
            out << "\n";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Colors saved to " + filePath);
}

void MainWindow::on_setBackgroundColor_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Wybierz kolor tła");

    if (!color.isValid()) return;


    QLayout* layout = ui->centralwidget->layout();
    int index = 0;

    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            colorArray[i][j][0] = color.red();
            colorArray[i][j][1] = color.green();
            colorArray[i][j][2] = color.blue();

            if (QPushButton* button = qobject_cast<QPushButton*>(layout->itemAt(index)->widget())) {
                button->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
                                          .arg(color.red())
                                          .arg(color.green())
                                          .arg(color.blue()));
            }
            ++index;
        }
    }


    HANDLE hSerial = CreateFileA("COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć portu COM5");
        return;
    }

    char buffer[32];
    sprintf(buffer, "999,%d,%d,%d\n", color.red(), color.green(), color.blue());
    DWORD bytesWritten;
    WriteFile(hSerial, buffer, strlen(buffer), &bytesWritten, NULL);
    CloseHandle(hSerial);
}

void MainWindow::on_set_from_file_clicked()
{
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString filePath = appDirPath + "/colors.txt";


    HANDLE hSerial = CreateFileA("COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć portu COM5");
        return;
    }


    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file: " + file.errorString());
        CloseHandle(hSerial);
        return;
    }

    QTextStream in(&file);
    int i = 0, j = 0;


    while (!in.atEnd() && i < 16) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        QStringList rgbValues = line.split(" ", Qt::SkipEmptyParts);
        if (rgbValues.size() >= 3 && j < 16) {
            int r = rgbValues[0].toInt();
            int g = rgbValues[1].toInt();
            int b = rgbValues[2].toInt();

            colorArray[i][j][0] = r;
            colorArray[i][j][1] = g;
            colorArray[i][j][2] = b;


            if (QLayout* layout = ui->centralwidget->layout()) {
                int index = i * 16 + j;
                if (QPushButton* button = qobject_cast<QPushButton*>(layout->itemAt(index)->widget())) {
                    button->setStyleSheet(
                        QString("background-color: rgb(%1, %2, %3);").arg(r).arg(g).arg(b)
                        );
                }


                char buffer[32];
                sprintf(buffer, "%d,%d,%d,%d\n", index, r, g, b);

                DWORD bytesWritten;
                if (!WriteFile(hSerial, buffer, strlen(buffer), &bytesWritten, NULL)) {
                    QMessageBox::warning(this, "Error", "Błąd zapisu do portu COM");
                    break;
                }

                Sleep(25);
            }

            ++j;
            if (j == 16) {
                j = 0;
                ++i;
            }
        }
    }

    file.close();
    CloseHandle(hSerial);

    QMessageBox::information(this, "Success", "Colors loaded from file i wysłane do COM5");
}

