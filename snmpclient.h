#ifndef SNMPCLIENT_H
#define SNMPCLIENT_H

#include <QtCore/QString>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "customtypes.h"

class SnmpClient
{
public:
    SnmpClient();
    ~SnmpClient();
    void setIp(QString ip);
    void createPdu(int pduType, int maxRepetitions = 1);
    void clearResponsePdu();
    void addOid(const oid *someOid, size_t size);
    void addOid(const oid *someOid, size_t size, QString value, char type);
    netsnmp_variable_list *varList();
    bool sendRequest();
    bool setupSession(SessionType::Enum sessionType);
    bool openSession();
    void setTimeoutSaveConfig();
    void createPduFromResponse(int pduType);
private:
    netsnmp_session *mSnmpSession;
    netsnmp_session  mBaseSession;
    netsnmp_pdu *mPdu;
    netsnmp_pdu *mResponsePdu;
    QString mIp;
};

#endif // SNMPCLIENT_H