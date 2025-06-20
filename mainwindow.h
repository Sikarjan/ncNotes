#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QList>
#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QPushButton>
#include <QTextEdit>
#include <QListView>
#include <QItemSelectionModel>
#include <QCloseEvent>
#include <QSettings>
#include <QMenu>
#include <QFileSystemWatcher>

#include "notemodel.h"
#include "settingdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void on_actionAdd_Instance_triggered();
    void updateEditor(const QItemSelection &selection,const QItemSelection &deselection);
    void on_actionQuit_triggered();
    void noteChanged();
    void saveNote();
    void saveAll(bool exit = false);
    void addNote();
    void renameNote();
    void deleteNote();
    void showSettings();
    void fileChanged(const QString &file);

private:
    Ui::MainWindow *ui;
    QUrl dir;
    QFileSystemWatcher watcher;
    QList<QUrl> instancelList;
    NoteModel *model;
    int changedNotes;
    SettingDialog *setDiag;
    QMap<QString, QDateTime> previousFileState;

    void addInstance(QUrl url);
    void detectChanges(const QString& path);
    QMap<QString, QDateTime> scanDirectory(const QString& path);
};
#endif // MAINWINDOW_H
