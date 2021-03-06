#include "signaturefilesmodel.h"
#include <redasm/redasm.h>
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>

SignatureFilesModel::SignatureFilesModel(REDasm::DisassemblerAPI *disassembler, QObject *parent): QAbstractListModel(parent), m_disassembler(disassembler)
{
    QDirIterator it(QString::fromStdString(REDasm::makeSignaturePath(std::string())), {"*.json"}, QDir::Files);

    while(it.hasNext())
    {
        QString sigpath = it.next();
        std::string sigid = QFileInfo(sigpath).baseName().toStdString();
        m_signaturefiles.push_back({ sigid, REDasm::makeSignaturePath(sigid) + ".json" });
    }
}

const REDasm::SignatureDB *SignatureFilesModel::load(const QModelIndex &index)
{
    if(!m_loadedsignatures.contains(index.row()))
    {
        REDasm::SignatureDB sigdb;
        sigdb.load(m_signaturefiles[index.row()].second);
        m_loadedsignatures[index.row()] = sigdb;
    }

    return &m_loadedsignatures[index.row()];
}

const std::string &SignatureFilesModel::signatureId(const QModelIndex &index) const { return m_signaturefiles[index.row()].first;  }
const std::string& SignatureFilesModel::signaturePath(const QModelIndex &index) const { return m_signaturefiles[index.row()].second; }

void SignatureFilesModel::add(const std::string &sigid, const std::string &sigpath)
{
    this->beginInsertRows(QModelIndex(), m_signaturefiles.size(), m_signaturefiles.size());
    m_signaturefiles.push_back({sigid, sigpath});
    this->endInsertRows();
}

void SignatureFilesModel::mark(const QModelIndex &index)
{
    m_disassembler->loader()->signatures().insert(this->signatureId(index));
    emit dataChanged(this->index(index.row()), this->index(index.row(), this->columnCount()));
}

bool SignatureFilesModel::isLoaded(const QModelIndex &index) const
{
    const std::string& signature = m_signaturefiles[index.row()].first;
    auto it = m_disassembler->loader()->signatures().find(signature);
    return it != m_disassembler->loader()->signatures().end();
}

bool SignatureFilesModel::contains(const std::string &sigid) const
{
    for(const auto& item : m_signaturefiles)
    {
        if(sigid == item.first)
            return true;
    }

    return false;
}

QVariant SignatureFilesModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    if(index.column() == 0)
        return QString::fromStdString(m_signaturefiles[index.row()].first);

    if(index.column() == 1)
        return this->isLoaded(index) ? "YES" : "NO";

    return QVariant();
}

QVariant SignatureFilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if((orientation == Qt::Vertical) || (role != Qt::DisplayRole))
        return QVariant();

    if(section == 0)
         return "Name";
    if(section == 1)
         return "Loaded";

    return QVariant();
}

int SignatureFilesModel::rowCount(const QModelIndex &) const { return m_signaturefiles.length(); }
int SignatureFilesModel::columnCount(const QModelIndex&) const { return 2; }
