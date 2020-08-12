#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtGui>
#include <QMainWindow>

class QLabel;
class mainDialog;
class QAction;
class QMenuBar;
class QStatusBar;

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=nullptr);
    mainDialog *maindialog;
    QStatusBar *statusBar;

protected:

private slots:
    void open();
    void about();
    void printInfoToStatusBar(QString);

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    QMenuBar *menubar;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *aboutQtAction;
    QAction *openAction;
    QAction *aboutAction;
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    void closeEvent(QCloseEvent *);
};

#endif // WIDGET_H
