#ifndef DEVICETABLEMODEL_H
#define DEVICETABLEMODEL_H

#ifdef _MSC_VER
#include "../Info/switchinfo.h"
#include "../Info/dslaminfo.h"
#include "../Info/oltinfo.h"
#else
#include "Info/switchinfo.h"
#include "Info/dslaminfo.h"
#include "Info/oltinfo.h"
#endif

class DeviceTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DeviceTableModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool insertRow(int row, const QModelIndex &parent);
    bool removeRow(int row, const QModelIndex &parent);
    bool isModified();
    void setModified(bool state);

    bool load();
    bool save();

    QVector<DeviceInfo::Ptr> &deviceList();

    BoardTableModel *boardListModel(QModelIndex index);

    int inetVlan(QModelIndex index);
    int iptvVlan(QModelIndex index);

    QStringListModel *serviceProfileOltListModel(QModelIndex index);
    QStringListModel *multicastProfileOltListModel(QModelIndex index);

    bool getVlanTagFromDevice(QModelIndex index);
    bool getBoardListFromDevice(QModelIndex index);
    bool getProfilesFromDevice(QModelIndex index);

    QString error() const;

private:
    bool exist();
    bool backup();

    void readNextElement(QXmlStreamReader &reader);

    void parseSwitchElement(QXmlStreamReader &reader);
    void parseDslamElement(QXmlStreamReader &reader);
    void parseDslamBoardList(QXmlStreamReader &reader,
                             DeviceInfo::Ptr deviceInfo);
    void parseOltElement(QXmlStreamReader &reader);
    void parseOltProfileList(QXmlStreamReader &reader,
                             DeviceInfo::Ptr deviceInfo);

    void createSwitchElement(QXmlStreamWriter &writer,
                             const SwitchInfo::Ptr &deviceInfo);
    void createDslamElement(QXmlStreamWriter &writer,
                            const DslamInfo::Ptr &deviceInfo);
    void createOltElement(QXmlStreamWriter &writer,
                          const OltInfo::Ptr &deviceInfo);
    void createOltProfileList(QXmlStreamWriter &writer,
                              const OltProfileMap &profileMap, QString typeElem);

    bool mModified;
    QString mError;
    QString mDeviceListPath;
    QString mDeviceListBackupPath;
    QVector<DeviceInfo::Ptr> mList;
};

#endif // DEVICETABLEMODEL_H
