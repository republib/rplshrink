/*
 * Licence:
 * You can use and modify this file without any restriction.
 * There is no warranty.
 * You also can use the licence from http://www.wtfpl.net/.
 * The original sources can be found on https://github.com/republib.
*/


#include "mainwindow.hpp"
#include "ui_mainwindow.h"

/** @class MainWindow mainwindow.hpp "mainwindow.hpp"
 *
 * @brief Implements the main window of the application.
 *
 * Administrates a form with all input data and a listbox for logging output.
 *
 * The button "convert" stars the conversion with the given data.
 */

/**
 * @brief Constructor.
 *
 * @param parent        NULL or the parent (who destroys the objects at the end)
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_converter(NULL)
{
    ui->setupUi(this);
    QStringList defaultX;
    defaultX.append("*");
    defaultX.append("1024");
    defaultX.append("1920");
    defaultX.append("800");
    QStringList defaultY;
    defaultY.append("*");
    defaultY.append("768");
    defaultY.append("1050");
    defaultY.append("600");
    QStringList quality;
    quality.append("70");
    quality.append("85");
    quality.append("100");
    QStringList target;
    target.append("normal");
    target.append("mini");
    target.append("1024");
    target.append("1920");
    ui->comboLandscapeX->addItems(defaultX);
    ui->comboLandscapeY->addItems(defaultY);
    ui->comboPortraitX->addItems(defaultX);
    ui->comboPortraitY->addItems(defaultY);
    ui->comboSquareX->addItems(defaultX);
    ui->comboQuality->addItems(quality);
    ui->comboTarget->addItems(target);
    ui->comboLandscapeX->setCurrentText("1024");
    ui->comboLandscapeY->setCurrentText("*");
    ui->comboPortraitX->setCurrentText("*");
    ui->comboPortraitY->setCurrentText("768");
    ui->comboSquareX->setCurrentText("768");
    ui->comboQuality->setCurrentText("70");
    ui->comboTarget->setCurrentText("normal");
    ui->buttonStop->hide();
}

/**
 * @brief Destructor
 */
MainWindow::~MainWindow()
{
    delete ui;
    delete m_converter;
}

/**
 * @brief Logs a message
 *
 * @param message   the message to log
 * @return          <code>true</code>
 */
bool MainWindow::log(const QString& message)
{
    ui->listWidget->addItem(message);
    return true;
}

/**
 * @brief Handles the button click on "convert".
 */
void MainWindow::on_buttonConvert_clicked()
{
    ui->buttonConvert->hide();
    ui->buttonStop->show();
    delete m_converter;
    m_converter = new Converter(ui->comboSourceDir->currentText(),
        ui->comboLandscapeX->currentText(),
        atol(ui->comboLandscapeX->currentText().toLatin1().constData()),
        atol(ui->comboLandscapeY->currentText().toLatin1().constData()),
        atol(ui->comboPortraitX->currentText().toLatin1().constData()),
        atol(ui->comboPortraitY->currentText().toLatin1().constData()),
        atol(ui->comboSquareX->currentText().toLatin1().constData()),
        atol(ui->comboQuality->currentText().toLatin1().constData()),
        this);
    // start the thread:
    m_converter->start();
}

/**
 * @brief Handles the event "thread changed".
 *
 * @param state     the new state of the thread
 * @param info      info about the new state. Not used
 */
void MainWindow::on_threadStateChanged(Converter::State state,
                                       const QString&)
{
    switch(state){
    case Converter::STATE_READY:
        ui->buttonConvert->show();
        ui->buttonStop->hide();
        //ui->statusBar->showMessage(info);
        break;
    case Converter::STATE_SUB_TASK_STOPPED:
        //ui->statusBar->showMessage(info);
        break;
    case Converter::STATE_STARTING:
    default:
        break;
    }

}
/**
 * @brief Handles the click on the button "stop".
 */
void MainWindow::on_buttonStop_clicked()
{
    m_converter->stop();
    ui->buttonConvert->show();
    ui->buttonStop->hide();
}

/**
 * @brief Handles the click on the button "file select".
 *
 * Shows a selection dialog for directories and sets the source directory
 * onto the selected directory.
 */
void MainWindow::on_buttonFileSelect_clicked()
{
    QFileDialog selection;
    selection.setFileMode(QFileDialog::DirectoryOnly);
    QString dir = ui->comboSourceDir->currentText();
    if (! dir.isEmpty())
        selection.setDirectory(dir);
    if (selection.exec())
        ui->comboSourceDir->setCurrentText(selection.selectedFiles().at(0));
}


