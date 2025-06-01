#ifndef NOTEMODEL_H
#define NOTEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QList>
#include <QStringList>

class NoteModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit NoteModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QStringList getData(const QModelIndex &index);
    bool setNote(const QModelIndex &index, QString note);
    bool setName(const QModelIndex &index, QString &name, QString &path);
    void append(const QString& name, const QString& note, const QString& path, const QString& hasChanged = "false");
    void insert(const QString& name, const QString& path);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool hasChanged(const int &index);
    void noteChanged(const int &index, const bool &change);

signals:
    void clicked();

private:
    QList<QStringList> mNotes;
    QString mName;
    QString mNote;
    QString mPath;
};

#endif // NOTEMODEL_H
