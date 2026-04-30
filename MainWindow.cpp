#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "MuseumWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Creating our OpenGL museum widget
    MuseumWidget *museumWidget = new MuseumWidget(this);

    // Put the OpenGL widget in the center of the main window
    setCentralWidget(museumWidget);

    setWindowTitle("Interactive 3D Art Museum");
}

MainWindow::~MainWindow()
{
    delete ui;
}
