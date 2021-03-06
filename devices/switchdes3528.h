#ifndef SWITCHDES3528_H
#define SWITCHDES3528_H

#include "switch.h"

class SwitchDes3528 : public Switch
{
    Q_OBJECT
public:
    explicit SwitchDes3528(QObject *parent = 0);
    SwitchDes3528(QString name, QString ip, QObject *parent = 0);
    explicit SwitchDes3528(Device *source, QObject *parent = 0);

    DeviceModel::Enum deviceModel() const;
    int countPorts();
    int sizePortBitArray();
    SwitchPort::Ptr createPort(QObject *parentModel);

    int  maxLengthPortDescription();
    bool setPortDescription(long snmpPortIndex, QString description);
};

#endif // SWITCHDES3528_H
