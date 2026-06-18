#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "markdownhighlighter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    model = new NoteModel(this);
    setDiag = new SettingDialog(this);
    changedNotes = 0;

    ui->editor->setFont(QFont("Courier New", 13));

    new MarkdownHighlighter(ui->editor->document());

    // for development only
    dir = QUrl("/Users/flo/Documents/Cloud/Notes");
    addInstance(dir);
    watcher.addPath(dir.path());
    previousFileState = scanDirectory(dir.toString());
    pollingTimer = new QTimer(this);

    connect(&watcher, SIGNAL(directoryChanged(QString)), this, SLOT(fileChanged(QString)));
    connect(pollingTimer, &QTimer::timeout, this, &MainWindow::pollDirectoryChanges);
    connect(ui->editor, SIGNAL(textChanged()), this, SLOT(noteChanged()));
    connect(setDiag, SIGNAL(newFont(QFont)), ui->editor, SLOT(setCurrentFont(QFont)));

    pollingTimer->start(5000);

    // Context menu for TreeView
    ui->noteView->addAction(tr("&Rename"), this, SLOT(renameNote()));
    ui->noteView->addAction(tr("&Delete"), this, SLOT(deleteNote()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if(changedNotes != 0){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("Unsaved changes in ncNotes"));
        msgBox.setInformativeText(tr("There are %n note(s) unsaved. Do you want to save them?", "", changedNotes));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
           case QMessageBox::Save:
               e->ignore();
               saveAll(true);
               break;
           case QMessageBox::Discard:
               e->accept();
               break;
           case QMessageBox::Cancel:
               e->ignore();
               break;
           default:
               e->accept();
               break;
         }
    }else {
        e->accept();
    }
}

void MainWindow::saveAll(bool exit){
    for(int i=0;i<model->rowCount(); i++){
        if(model->hasChanged(i)){
            QModelIndex index = model->index(i,0);
            ui->noteView->setCurrentIndex(index);
            saveNote();

            if(changedNotes == 0){
                if(exit){
                    this->close();
                }else{
                    return;
                }
            }
        }
    }
}

void MainWindow::addNote()
{
    bool ok;
    QString noteName = QInputDialog::getText(this,
         tr("Add Note"),
         tr("Note Name"),
         QLineEdit::Normal,
         tr("New Note"),
         &ok);
    if(ok && !noteName.isEmpty()){
        QString path = dir.path()+"/"+noteName+".md";
        QFile file(path);
        if(file.open(QIODevice::ReadOnly|QIODevice::Text)){
            QMessageBox msg;
            msg.setIcon(QMessageBox::Warning);
            msg.setText(tr("A note with this name already exists."));
            msg.exec();
            return;
        }
        model->insert(noteName, path);
        QModelIndex index = model->index(model->rowCount()-1,0);
        ui->noteView->setCurrentIndex(index);
        ui->editor->setFocus();
        emit model->dataChanged(index,index);
        changedNotes++;
    }
}

void MainWindow::renameNote()
{
    QModelIndex index = ui->noteView->currentIndex();
    if(!index.isValid()){
        return;
    }

    QString oldName = model->data(index).toString();
    bool ok;
    QString name = QInputDialog::getText(this,
        tr("New Note Name"),
        tr("New Name"), QLineEdit::Normal,
        oldName,
        &ok
    );

    if (!ok || name.isEmpty()){
        return;
    }

    QStringList tNote = model->getData(index);
    QString newPath = dir.path()+"/"+name+".md";

    QFile file;
    if(file.rename(tNote[2], newPath)){
        model->setName(index, name, newPath);
    }else{
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("Renaming faild!"));
        msgBox.exec();
    }
}

void MainWindow::deleteNote() {
    QModelIndex index = ui->noteView->currentIndex();
    if(!index.isValid()){
        return;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Move to trash");
    msgBox.setText(tr("Do you want to move this note to trash?"));
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::No){
      return;
    }

    watcher.blockSignals(true);
    QStringList tNote = model->getData(index);
    QFile file(tNote[2]);
    if(file.moveToTrash()){
        model->removeRow(index.row());
    }
    previousFileState.remove(file.fileName());
    watcher.blockSignals(false);
}

void MainWindow::showSettings()
{
    setDiag->cFont = ui->editor->currentFont();
    setDiag->show();
}

void MainWindow::fileChanged(const QString& file)
{
    qDebug() << "File Change detected";
    detectChanges(file);
}

void MainWindow::on_actionAdd_Instance_triggered()
{
    // Create better dialog to also add a name for the instance
    dir = QFileDialog::getExistingDirectoryUrl(this, tr("Select Instance"), QUrl(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)), QFileDialog::ShowDirsOnly);

    if(!dir.isValid()){
        return;
    }

    // Check if instance is already in use
    if(instancelList.contains(dir)){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("This instance already exists"));
        msgBox.exec();
        return;
    }

    // Add a new instance to the instanceViewer
    instancelList.append(dir);
    addInstance(dir);
}

void MainWindow::updateEditor(const QItemSelection& selection, const QItemSelection &deselection)
{
    // Saving changes to model, not to files!
    if(!deselection.indexes().isEmpty()){
        model->setNote(deselection.indexes().first(), ui->editor->toHtml());
    }
    if(selection.indexes().isEmpty()){
        ui->editor->blockSignals(true);
        ui->editor->clear();
        ui->editor->blockSignals(false);
        return;
    }

    QVariant tmpNote = model->data(selection.indexes().first(), Qt::EditRole);
    ui->editor->blockSignals(true);
    ui->editor->setText(tmpNote.toString());
    ui->editor->blockSignals(false);

    // Set save option correctly
    ui->actionSave->setEnabled(model->hasChanged(selection.indexes().first().row()));
}

void MainWindow::addInstance(QUrl url)
{
    QDir mDir = url.path();
    QStringList nameFilters;
    nameFilters << "*.txt" << "*.md";

    QFileInfoList notes = mDir.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    for(int i=0;i<notes.size();i++){
        QString tmpNote;
        QFileInfo fileInfo = notes.at(i);
        QFile note(fileInfo.filePath());

        if(note.open(QIODevice::ReadOnly)){
            QTextStream in(&note);
            tmpNote = in.readAll();
        }else{
            tmpNote = (tr("Error reading file."));
        }
        note.close();
        fileLoadTimes[fileInfo.filePath()] = fileInfo.lastModified();
        model->append(fileInfo.baseName(),tmpNote, fileInfo.filePath());
    }

    ui->noteView->setModel(model);

    connect(ui->noteView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this,
            SLOT(updateEditor(QItemSelection,QItemSelection)));
}

void MainWindow::detectChanges(const QString &path = "")
{
    pollingTimer->stop();

    QString actualPath = path.isEmpty() ? dir.path() : path;
    QMap<QString, QDateTime> currentState = scanDirectory(actualPath);
    // Vergleiche vorher/nachher
    for (const QString &file : currentState.keys()) {
        QString tmpNote;
        QFileInfo fileInfo(path+"/"+file);
        QFile note(fileInfo.filePath());

        // New file added externally
        if (!previousFileState.contains(file)) {
            if(note.open(QIODevice::ReadOnly)){
                QTextStream in(&note);
                tmpNote = in.readAll();
            }else{
                tmpNote = (tr("Error reading file."));
            }
            note.close();

            model->append(fileInfo.baseName(),tmpNote, fileInfo.filePath());
            QModelIndex index = model->index(model->rowCount()-1,0);
            emit model->dataChanged(index,index);
        } else if (previousFileState[file] != currentState[file]) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("File changed externally"));
            msgBox.setInformativeText(tr("The file '%1' was modified. Reload?").arg(file));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            if (msgBox.exec() == QMessageBox::Yes) {
                reloadNote(dir.path() + "/" + file);
            }
        }
    }

    for (const QString &file : previousFileState.keys()) {
        if (!currentState.contains(file)) { 
            qDebug() << "Datei gelöscht:" << file;
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Note deleted"));
            msgBox.setText(tr("Note ")+file+tr(" was removed. Do you want to keep it in the editor?"));
            msgBox.setStandardButtons(QMessageBox::Yes);
            msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            if(msgBox.exec() == QMessageBox::Yes){
                // Modell-Eintrag finden und aktualisieren
                for (int i = 0; i < model->rowCount(); i++) {
                    QStringList noteData = model->getData(model->index(i, 0));
                    if (noteData[2] == dir.path() + "/" + file) {
                        model->removeRow(i);

                        // Falls diese Notiz gerade im Editor angezeigt wird
                        if (ui->noteView->currentIndex() == model->index(i, 0)) {
                            ui->editor->setText("");
                        }
                        return;
                    }
                }
            }

            // Need to detect which row the deleted note was and remove it from the model
        }
    }

    // Zustand aktualisieren
    previousFileState = currentState;

    pollingTimer->start(5000);
}

QMap<QString, QDateTime> MainWindow::scanDirectory(const QString &path)
{
    QMap<QString, QDateTime> state;
    QDir dir(path);
    for (const QFileInfo &file : dir.entryInfoList(QDir::Files)) {
        state[file.fileName()] = file.lastModified();
    }
    return state;
}

void MainWindow::pollDirectoryChanges()
{
    QMap<QString, QDateTime> currentState = scanDirectory(dir.path());
    if (currentState != previousFileState) {
        detectChanges(dir.path());
    }
}

void MainWindow::reloadNote(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error reloading file:" << filePath;
        return;
    }

    QTextStream in(&file);
    QString newContent = in.readAll();
    file.close();

    // Modell-Eintrag finden und aktualisieren
    for (int i = 0; i < model->rowCount(); i++) {
        QStringList noteData = model->getData(model->index(i, 0));
        if (noteData[2] == filePath) {
            model->setNote(model->index(i, 0), newContent);

            // Falls diese Notiz gerade im Editor angezeigt wird
            if (ui->noteView->currentIndex() == model->index(i, 0)) {
                ui->editor->blockSignals(true);
                ui->editor->setText(newContent);
                ui->editor->blockSignals(false);
            }

            // Aktualisierte Modifikationszeit speichern
            QFileInfo info(filePath);
            fileLoadTimes[filePath] = info.lastModified();
            previousFileState[info.fileName()] = info.lastModified();

            emit model->dataChanged(model->index(i, 0), model->index(i, 0));
            return;
        }
    }
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::noteChanged()
{
    QModelIndex index = ui->noteView->currentIndex();
    if(model->hasChanged(index.row())){
        return;
    }

    model->noteChanged(index.row(), true);
    ui->actionSave->setEnabled(true);
    ui->actionSave_all->setEnabled(true);
    emit model->dataChanged(index,index);
    changedNotes++;
}

void MainWindow::saveNote(){
    QModelIndex index = ui->noteView->currentIndex();
    QStringList tmpNote = model->getData(index);

    // Check if file has changed in the directory
    QFileInfo currentInfo(tmpNote[2]);
    if (fileLoadTimes.contains(tmpNote[2]) &&
        currentInfo.lastModified() > fileLoadTimes[tmpNote[2]]) {

        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("This note has been modified externally."));
        msgBox.setInformativeText(tr("Do you want to overwrite the external changes?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) {
            return; // Speichern abbrechen
        }
    }

    watcher.blockSignals(true);
    QFile file(tmpNote[2]);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        qDebug() << "error saving" << tmpNote[2];
        return;
    }

    QTextStream out(&file);
    out << ui->editor->toPlainText() << "\n";
    file.close();

    QFileInfo info(file);
    previousFileState[info.fileName()] = info.lastModified();

    watcher.blockSignals(false);

    model->noteChanged(index.row(), false);
    emit model->dataChanged(index,index);
    changedNotes--;

    ui->actionSave->setEnabled(false);
    if(changedNotes == 0){
        ui->actionSave_all->setEnabled(false);
    }
}

