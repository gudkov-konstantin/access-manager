#ifndef SWITCHINFO_H
#define SWITCHINFO_H

#include "deviceinfo.h"

#ifdef _MSC_VER
#include "../snmpclient.h"
#else
#include "snmpclient.h"
#endif

class SwitchInfo : public DeviceInfo
{
    Q_OBJECT
public:
    explicit SwitchInfo(QObject *parent = 0);
    SwitchInfo(QString name, QString ip, QObject *parent = 0);

    int inetVlanTag() const;
    void setInetVlanTag(int vlanTag);

    int iptvVlanTag() const;
    void setIptvVlanTag(int vlanTag);

    DeviceType::Enum deviceType() const;

    bool getServiceDataFromDevice();

    bool saveConfig();

    typedef QSharedPointer<SwitchInfo> Ptr;

private:
    int mInetVlanTag;
    int mIptvVlanTag;
};

#endif // SWITCHINFO_H
