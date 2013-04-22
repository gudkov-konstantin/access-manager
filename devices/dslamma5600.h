#ifndef DSLAMMA5600_H
#define DSLAMMA5600_H

#include "dslam.h"

class DslamMa5600 : public Dslam
{
    Q_OBJECT
public:
    explicit DslamMa5600(QObject *parent = 0);
    DslamMa5600(QString name, QString ip, QObject *parent = 0);
    DslamMa5600(Device *source, QObject *parent = 0);

    DeviceModel::Enum deviceModel() const;
    int countPorts(BoardType::Enum boardType);
    long snmpInterfaceIndex(BoardType::Enum boardType, int slot, int port);
    XdslPort::Ptr createPort(BoardType::Enum boardType, int boardIndex,
                             int portIndex, QObject *parent);
};

#endif // DSLAMMA5600_H