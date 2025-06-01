#include "notemodel.h"

NoteModel::NoteModel(QObject *parent)
    : QAbstractListModel{parent}
{

}

int NoteModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mNotes.size();
}

QVariant NoteModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()){
        return QVariant();
    }

    if(role==Qt::DisplayRole){
        return mNotes[index.row()].at(0);
    }
    if(role==Qt::EditRole){
        return mNotes[index.row()].at(1);
    }
    return QVariant();
}

QStringList NoteModel::getData(const QModelIndex &index)
{
    if(!index.isValid()){
        return QStringList({"", "", "", ""});
    }

    return mNotes[index.row()];
}

bool NoteModel::setNote(const QModelIndex &index, QString note)
{
    QStringList tNote = mNotes.at(index.row());
    if(tNote.isEmpty()){
        return false;
    }
    tNote[1] = note;
    mNotes[index.row()] = tNote;

    return true;
}

bool NoteModel::setName(const QModelIndex &index, QString &name, QString &path)
{
    QStringList tNote = mNotes.at(index.row());
    if(tNote.isEmpty()){
        return false;
    }
    tNote[0] = name;
    tNote[2] = path;
    mNotes[index.row()] = tNote;

    return true;
}

void NoteModel::append(const QString &name, const QString &note, const QString &path, const QString &hasChanged)
{
    QStringList tNote = {name, note, path, hasChanged};
    mNotes.append(tNote);
}

void NoteModel::insert(const QString &name, const QString &path)
{
    append(name+"*", "", path,"true");
    // missing sorting model by name
}

bool NoteModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row + count > rowCount(parent))
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        mNotes.removeAt(row);
    }
    endRemoveRows();
    return true;
}

bool NoteModel::hasChanged(const int &index)
{
    QStringList tNote = mNotes.at(index);
    if(tNote[3] == "false"){
        return false;
    }else{
        return true;
    }
}

void NoteModel::noteChanged(const int &index, const bool &change)
{
    QStringList tNote = mNotes.at(index);

    if(change){
        tNote[0] = tNote[0]+"*";
        tNote[3] = "true";
    }else{
        tNote[0] = tNote[0].left(tNote[0].size()-1);
        tNote[3] = "false";
    }
    mNotes[index] = tNote;
}
