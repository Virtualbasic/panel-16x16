#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_colorButton_clicked(int index);
    void on_saveToFile_clicked(); // Nowy slot
    void on_setBackgroundColor_clicked();
    void on_set_from_file_clicked();
private:
    Ui::MainWindow *ui;
    QColor selectedColor;

    std::vector<std::vector<std::vector<int>>> colorArray;
};

#endif
