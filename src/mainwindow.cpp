#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fileSelected(FALSE),
    fileSize(0)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_browse_clicked()
{
    QString path;

    path = QFileDialog::getOpenFileName(
        this,
        "Choose a file to split",
        QString::null,
        tr("Audio File (*.mp3)"));

    ui->lineEdit_fileName->setText( path );
}

void MainWindow::on_pushButton_browseDir_clicked()
{
    QString path;

    path = QFileDialog::getExistingDirectory(
        this,
        "Open directory to place split mp3s",
        QString::null,
        QFileDialog::ShowDirsOnly);

    ui->lineEdit_outputDirectory->setText( path );
}




void MainWindow::on_pushButton_split_clicked()
{
    ui->label_status->setText("Splitting");
    splitFile(ui->spinBox_splitSize->value()*1024, ui->lineEdit_fileName->text(), ui->lineEdit_outputDirectory->text() + QLatin1Char('/') + ui->lineEdit_outputFilename->text());
    ui->progressBar->setValue(100);
    ui->label_status->setText("Done!");
    QApplication::beep();

}




void MainWindow::on_spinBox_splitSize_valueChanged(int arg1)
{
    ui->horizontalSlider_fileSize->setValue(arg1);

    updateInfoBox();
}


void MainWindow::on_lineEdit_outputFilename_textChanged(const QString &arg1)
{
    updateInfoBox();
}

void MainWindow::updateInfoBox(void)
{
    int numSplitFiles = 0;

    if(fileSelected)
    {
        numSplitFiles = fileSize / (ui->spinBox_splitSize->value()*1024);
        if(fileSize % (ui->spinBox_splitSize->value()*1024))
            numSplitFiles++;

        QString text_integer;
        text_integer.setNum(numSplitFiles);
        ui->label_noOutFiles->setText(text_integer);
    }
    else
        ui->label_noOutFiles->setText("");

    ui->label_outputFilename_2->setText( (ui->lineEdit_outputFilename->text()) + "01.mp3");
}

void MainWindow::on_horizontalSlider_fileSize_valueChanged(int value)
{
      ui->spinBox_splitSize->setValue(value);
}

void MainWindow::on_lineEdit_fileName_textChanged(const QString &arg1)
{
    FILE *check;

    QByteArray byteArray = arg1.toUtf8();
    char * cString = byteArray.data();

    check = fopen(cString, "rb");
    if(check == NULL)
    {
        printf("file '%s' does not exist..\n",cString);
        ui->label_inputSize->setText("");
        fileSelected = FALSE;
        fileSize = 0;
        updateInfoBox();
        return;
    }
    fileSelected = TRUE;

    fseek(check, 0, SEEK_END);
    fileSize = ftell(check);

    QString text_integer;
    text_integer.setNum(fileSize);
    ui->label_inputSize->setText(text_integer + " bytes");
    updateInfoBox();
}




int MainWindow::splitFile(int splitSize, QString inputFile, QString outputFile)
{
    int totalSize = 0;
    int written = 0;
    int fileNum = 0;
    FILE *in;

    QByteArray byteArray = inputFile.toUtf8();
    char * cString = byteArray.data();

    in = fopen(cString, "rb");
    if(in == NULL)
    {
        printf("file '%s' does not exist..\n",cString);
        return(-1);
    }

    fseek(in, 0, SEEK_END);
    totalSize = ftell(in);
    rewind(in);

    byteArray = outputFile.toUtf8();
    cString = byteArray.data();

    //figure progress bar steps
    int aproxPieces = totalSize / splitSize;

    fileNum = 1;
    for(int i=0; i<totalSize; i+=written)
    {
        ui->progressBar->setValue(((fileNum+1) *100)/aproxPieces);
        written = write(in, splitSize, cString, fileNum);
        if(written < 0)
        {
            printf("error writing file %d\n\n", fileNum);
            return(written);
        }
        fileNum++;
    }


    return 0;
}

int MainWindow::write(FILE *in, int splitSize, char *outFilename, int fileNum)
{
    FILE *out = NULL;
    int sizeRead = 0;
    int outnameLen = 0;
    unsigned char *buf = NULL;
    char outFilenameNum[4096] = {0};
    char idx[10] = {0};
    int written = 0;
    int tmpRead = 0;

    buf = (unsigned char*)malloc(splitSize);
    if(buf == NULL)
    {
        printf("error mallocing %d\n", splitSize);
        return -2;
    }

    strncpy(outFilenameNum, outFilename, 4000);
    outnameLen = strlen(outFilename);
#ifdef WIN32
//    sprintf(idx, "-%03d.mp3", fileNum);
        sprintf(idx, "%02d.mp3", fileNum);
#else
//    snprintf(idx, 10, "-%03d.mp3", fileNum);
        snprintf(idx, 10, "%02d.mp3", fileNum);
#endif
    strncat(outFilenameNum+outnameLen, idx, 10);

    out = fopen(outFilenameNum, "wb");
    if(out == NULL)
    {
        printf("error opening '%s'\n", outFilenameNum);
        return -3;
    }
    sizeRead = fread(buf, 1, splitSize, in);

    written = fwrite(buf, 1, sizeRead, out);

    if(written != sizeRead)
    {
        printf("error writing to file %d\n", fileNum);
        return -4;
    }

    if(sizeRead < splitSize) // we reached end of file
    {
        return sizeRead;
    }

    memset(buf,0,splitSize); // reset buf and start reading into it until header is reached
    {

        int loop = 1;
        int read = 0;

        unsigned char a;
        unsigned char b;

        tmpRead = 0;

        tmpRead += fread(&buf[tmpRead], 1, 1, in);

        if(tmpRead != 1)
        {
            loop = 0; //break and write rest of file
        }
        while(loop)
        {
            read = fread(&buf[tmpRead], 1, 1, in);
            if(read != 1)
            {
                break; //break and write rest of file
            }
            tmpRead += read;

           // a = buf[tmpRead-2];
            //b = buf[tmpRead-1];
            if((buf[tmpRead-2] == 0xff) && (buf[tmpRead-1] == 0xfb))// found header, back up 2 and exit
            {
                fseek(in, -2, SEEK_CUR);
                tmpRead -= 2;
                break;
            }
            if(tmpRead >= splitSize) //end of buffer, we're not finding mp3 header for some reason, just exit -- will fuck up file
            {
                break;
            }
        }
        //write out rest
        fwrite(buf, 1, tmpRead, out);
    }
    fclose(out);

    return(splitSize+tmpRead);

}
