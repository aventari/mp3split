#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    bool fileSelected;
    int fileSize;

    void updateInfoBox(void);
    int splitFile(int splitSize, QString inputFile, QString outputFile);
    int write(FILE *in, int splitSize, char *outFilename, int fileNum);

private slots:
    void on_pushButton_browse_clicked();
    void on_pushButton_split_clicked();
    void on_pushButton_browseDir_clicked();
    void on_spinBox_splitSize_valueChanged(int arg1);
    void on_horizontalSlider_fileSize_valueChanged(int value);
    void on_lineEdit_fileName_textChanged(const QString &arg1);
    void on_lineEdit_outputFilename_textChanged(const QString &arg1);
};

#endif // MAINWINDOW_H
