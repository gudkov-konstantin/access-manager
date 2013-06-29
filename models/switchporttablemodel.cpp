#include "switchporttablemodel.h"

#include <basicdialogs.h>
#include <constant.h>
#include <customsnmpfunctions.h>
#include <ports/switchportdes3526.h>
#include <ports/switchportdes3528.h>
#include <ports/switchportdes3550.h>

// Columns
// DisplayRole
// 0 - number
// 1 - operation state
// 2 - speed/duplex
// 3 - description

SwitchPortTableModel::SwitchPortTableModel(Switch::Ptr parentDevice,
                                           QObject *parent) :
    QAbstractTableModel(parent),
    mParentDevice(parentDevice)
{
    createList();

    mIptvMultVlanTag = 0;

    int countBit;

    if ((mParentDevice->deviceModel() == DeviceModel::DES3526)
            || (mParentDevice->deviceModel() == DeviceModel::DES3528)) {
        countBit = 32;
    } else if (mParentDevice->deviceModel() == DeviceModel::DES3550) {
        countBit = 56;
    } else {
        countBit = 32;
    }

    mInetVlanAllMember.fill(false, countBit);
    mInetVlanUntagMember.fill(false, countBit);
    mIptvUnicastVlanAllMember.fill(false, countBit);
    mIptvUnicastVlanUntagMember.fill(false, countBit);
    mMulticastVlanMember.fill(false, countBit);
}

int SwitchPortTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return mList.size();
}

int SwitchPortTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 4;
}

QVariant SwitchPortTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        } else {
            return int(Qt::AlignCenter | Qt::AlignVCenter);
        }
    } else if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0) {
            return mList.at(index.row())->index();
        } else if (index.column() == 1) {
            return SwitchPortState::toString(mList.at(index.row())->state());
        } else if (index.column() == 2) {
            return mList.at(index.row())->speedDuplex();
        } else if (index.column() == 3) {
            return mList.at(index.row())->description();
        }
    } else if ((role == Qt::BackgroundColorRole) && (index.internalId() == 0)) {
        if (mList.at(index.row())->state() == SwitchPortState::Down) {
            return QBrush(QColor(223, 255, 252));
        } else if (mList.at(index.row())->state() == SwitchPortState::Up) {
            return QBrush(QColor(200, 255, 200));
        }
    }

    return QVariant();
}

QVariant SwitchPortTableModel::headerData(int section, Qt::Orientation orientation,
                                          int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (section == 0) {
            return SwitchPortTableModelStrings::Number;
        } else if (section == 1) {
            return SwitchPortTableModelStrings::State;
        } else if (section == 2) {
            return SwitchPortTableModelStrings::SpeedDuplex;
        } else if (section == 3) {
            return SwitchPortTableModelStrings::Desc;
        }
    } else if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignCenter | Qt::AlignVCenter);
    } else if (role == Qt::FontRole) {
        QFont font(qApp->font());
        font.setBold(true);
        return font;
    }

    return QVariant();
}

Qt::ItemFlags SwitchPortTableModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void SwitchPortTableModel::sort(int column, Qt::SortOrder order)
{
    beginResetModel();

    qSort(mList.begin(), mList.end(),
              [&](const SwitchPort::Ptr first,
    const SwitchPort::Ptr second) -> bool {
        if (column == 0) {
            return order == Qt::AscendingOrder ? (first->index() < second->index())
                                               : (first->index() > second->index());
        } else if (column == 1) {
            return order == Qt::AscendingOrder ? (first->state() < second->state())
                                               : (first->state() > second->state());
        } else if (column == 2) {
            return order == Qt::AscendingOrder ? (first->speedDuplex() < second->speedDuplex())
                                               : (first->speedDuplex() > second->speedDuplex());
        } else if (column == 3) {
            return order == Qt::AscendingOrder ? (first->description() < second->description())
                                               : (first->description() > second->description());
        }

        return false;
    });

    endResetModel();
}

bool SwitchPortTableModel::memberMulticastVlan(int port)
{
    return mMulticastVlanMember.at(port - 1);
}

bool SwitchPortTableModel::setMemberMulticastVlan(int port, bool value)
{
    if (!getMulticastVlanSettings())
        return false;

    if (mMulticastVlanMember.at(port - 1) == value) {
        if (value) {
            mError = QString::fromUtf8("Ошибка: Порт %1 уже является членом Multicast влана.")
                     .arg(QString::number(port));
        } else {
            mError = QString::fromUtf8("Ошибка: Порт %1 не является членом Multicast влана.")
                     .arg(QString::number(port));
        }

        return false;
    } else {
        mMulticastVlanMember.setBit(port - 1, value);
    }

    QVector<OidPair> oidList;

    OidPair vlanMemberOid;
    if (mParentDevice->deviceModel() == DeviceModel::DES3526) {
        vlanMemberOid = createOidPair(Mib::swL2IGMPMulticastVlanMemberPortDES3526, 15, mIptvMultVlanTag);
    } else if (mParentDevice->deviceModel() == DeviceModel::DES3550) {
        vlanMemberOid = createOidPair(Mib::swL2IGMPMulticastVlanMemberPortDES3550, 15, mIptvMultVlanTag);
    } else {
        vlanMemberOid = createOidPair(Mib::swL2IGMPMulticastVlanMemberPortDES3528, 13, mIptvMultVlanTag);
    }

    oidList.push_back(vlanMemberOid);

    QList<QBitArray> arrayList;
    arrayList.push_back(mMulticastVlanMember);

    bool result = sendVlanSetting(oidList, arrayList, true);

    return result;
}

VlanState::Enum SwitchPortTableModel::memberInetVlan(int port)
{
    if (mInetVlanUntagMember.at(port - 1)) {
        return VlanState::Untag;
    } else if (mInetVlanAllMember.at(port - 1)) {
        return VlanState::Tag;
    } else {
        return VlanState::None;
    }
}

VlanState::Enum SwitchPortTableModel::memberIptvVlan(int port)
{
    if (mIptvUnicastVlanUntagMember.at(port - 1)) {
        return VlanState::Untag;
    } else if (mIptvUnicastVlanAllMember.at(port - 1)) {
        return VlanState::Tag;
    } else {
        return VlanState::None;
    }
}

bool SwitchPortTableModel::setMemberInternetService(int port)
{
    OidPair vlanMemberOid = createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->inetVlanTag());
    bool result = getUnicastVlanSettings(vlanMemberOid, mInetVlanAllMember, "Inet");

    if (!result)
        return false;

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->inetVlanTag());
    result = getUnicastVlanSettings(vlanMemberOid, mInetVlanUntagMember, "Inet");

    if (!result)
        return false;

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->iptvVlanTag());
    result = getUnicastVlanSettings(vlanMemberOid, mIptvUnicastVlanAllMember, "IPTV Unicast");

    if (!result) {
        return false;

    }

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->iptvVlanTag());
    result = getUnicastVlanSettings(vlanMemberOid, mIptvUnicastVlanUntagMember, "IPTV Unicast");

    if (!result) {
        return false;
    }

    if ((mInetVlanAllMember.at(port - 1) == true)
            && (mInetVlanUntagMember.at(port - 1) == true)) {
        mError = QString::fromUtf8("Ошибка: Порт %1 уже прописан под Интернет.")
                 .arg(QString::number(port));

        return false;
    } else {
        mInetVlanAllMember.setBit(port - 1, true);
        mInetVlanUntagMember.setBit(port - 1, true);
        mIptvUnicastVlanAllMember.setBit(port - 1, false);
        mIptvUnicastVlanUntagMember.setBit(port - 1, false);
    }

    QVector<OidPair> oidList;
    oidList.push_back(createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->iptvVlanTag()));
    oidList.push_back(createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->iptvVlanTag()));
    oidList.push_back(createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->inetVlanTag()));
    oidList.push_back(createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->inetVlanTag()));

    QList<QBitArray> arrayList;
    arrayList.push_back(mIptvUnicastVlanUntagMember);
    arrayList.push_back(mIptvUnicastVlanAllMember);
    arrayList.push_back(mInetVlanAllMember);
    arrayList.push_back(mInetVlanUntagMember);

    return sendVlanSetting(oidList, arrayList, false);
}

bool SwitchPortTableModel::setMemberInternetWithIptvStbService(int port)
{
    OidPair vlanMemberOid = createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->inetVlanTag());
    bool result = getUnicastVlanSettings(vlanMemberOid, mInetVlanAllMember, "Inet");

    if (!result)
        return false;

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->inetVlanTag());
    result = getUnicastVlanSettings(vlanMemberOid, mInetVlanUntagMember, "Inet");

    if (!result)
        return false;

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->iptvVlanTag());
    result = getUnicastVlanSettings(vlanMemberOid, mIptvUnicastVlanAllMember, "IPTV Unicast");

    if (!result)
        return false;

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->iptvVlanTag());
    result = getUnicastVlanSettings(vlanMemberOid, mIptvUnicastVlanUntagMember, "IPTV Unicast");

    if (!result)
        return false;

    if ((mInetVlanAllMember[port - 1] == true)
            && (mInetVlanUntagMember[port - 1] == false)
            && (mIptvUnicastVlanAllMember[port - 1] == true)
            && (mIptvUnicastVlanUntagMember[port - 1] == true)) {
        mError = QString::fromUtf8("Ошибка: Порт %1 уже прописан под Интернет + IPTV STB.")
                 .arg(QString::number(port));

        return false;
    } else {
        mInetVlanAllMember.setBit(port - 1, true);
        mInetVlanUntagMember.setBit(port - 1, false);
        mIptvUnicastVlanAllMember.setBit(port - 1, true);
        mIptvUnicastVlanUntagMember.setBit(port - 1, true);
    }

    QVector<OidPair> oidList;
    oidList.push_back(createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->inetVlanTag()));
    oidList.push_back(createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->inetVlanTag()));
    oidList.push_back(createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->iptvVlanTag()));
    oidList.push_back(createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->iptvVlanTag()));

    QList<QBitArray> arrayList;
    arrayList.push_back(mInetVlanAllMember);
    arrayList.push_back(mInetVlanUntagMember);
    arrayList.push_back(mIptvUnicastVlanAllMember);
    arrayList.push_back(mIptvUnicastVlanUntagMember);

    return sendVlanSetting(oidList, arrayList, false);
}

bool SwitchPortTableModel::updateInfoPort(int indexPort)
{
    QScopedPointer<SnmpClient> snmpClient(new SnmpClient());

    snmpClient->setIp(mParentDevice->ip());

    if (!snmpClient->setupSession(SessionType::ReadSession)) {
        mError = SnmpErrorStrings::SetupSession;
        return false;
    }

    if (!snmpClient->openSession()) {
        mError = SnmpErrorStrings::OpenSession;
        return false;
    }

    SwitchPort::Ptr currentPort = 0;

    auto it = mList.constBegin();
    auto end = mList.constEnd();
    for (; it != end; ++it) {
        if ((*it)->index() == indexPort) {
            currentPort = *it;
            break;
        }
    }

    if (!currentPort)
        return false;

    snmpClient->createPdu(SNMP_MSG_GET);

    currentPort->fillPdu(snmpClient.data());

    if (snmpClient->sendRequest()) {
        if (!currentPort->parsePdu(snmpClient.data())) {
            mError = SnmpErrorStrings::Parse;
            return false;
        } else {
            emit dataChanged(index(indexPort - 1, 1), index(indexPort - 1, 3));
        }
    } else {
        mError = snmpClient->error();
        return false;
    }

    return true;
}

bool SwitchPortTableModel::updateInfoAllPort()
{
    getVlanSettings();

    QScopedPointer<SnmpClient> snmpClient(new SnmpClient());

    snmpClient->setIp(mParentDevice->ip());

    if (!snmpClient->setupSession(SessionType::ReadSession)) {
        mError = SnmpErrorStrings::SetupSession;
        return false;
    }

    if (!snmpClient->openSession()) {
        mError = SnmpErrorStrings::OpenSession;
        return false;
    }

    int count = mList.size();

    for (int indexPort = 0; indexPort < count; ++indexPort) {
        snmpClient->createPdu(SNMP_MSG_GET);
        mList[indexPort]->fillPdu(snmpClient.data());

        if (snmpClient->sendRequest()) {
            if (!mList[indexPort]->parsePdu(snmpClient.data())) {
                mError = QString::fromUtf8("При получении информации по портам произошли ошибки.");
            } else {
                emit dataChanged(index(indexPort, 1), index(indexPort, 3));
            }

            snmpClient->clearResponse();
        } else {
            mError = QString::fromUtf8("При получении информации по портам произошли ошибки.");
            return false;
        }
    }

    return true;
}

bool SwitchPortTableModel::getVlanSettings()
{
    QString allErrors;

    OidPair vlanMemberOid = createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->inetVlanTag());
    bool result = getUnicastVlanSettings(vlanMemberOid, mInetVlanAllMember, "Inet");

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->inetVlanTag());
    result &= getUnicastVlanSettings(vlanMemberOid, mInetVlanUntagMember, "Inet");

    if (!result)
        allErrors += mError + "\n";

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticEgressPorts, 13, mParentDevice->iptvVlanTag());
    result = getUnicastVlanSettings(vlanMemberOid, mIptvUnicastVlanAllMember, "IPTV Unicast");

    vlanMemberOid = createOidPair(Mib::dot1qVlanStaticUntaggedPorts, 13, mParentDevice->iptvVlanTag());
    result &= getUnicastVlanSettings(vlanMemberOid, mIptvUnicastVlanUntagMember, "IPTV Unicast");

    if (!result)
        allErrors += mError + "\n";

    result = getMulticastVlanSettings();

    if (!result)
        allErrors += mError + "\n";

    if (!allErrors.isEmpty())
        BasicDialogs::error(0, BasicDialogStrings::Error, QString::fromUtf8("Произошла ошибка при получении информации о принадлежности к сервисным VLAN!"), allErrors);

    return true;
}

QString SwitchPortTableModel::error() const
{
    return mError;
}

void SwitchPortTableModel::createList()
{
    int count = mParentDevice->countPorts();
    mList.reserve(count);

    for (int i = 1; i <= count; ++i) {
        SwitchPort::Ptr portInfo = 0;

        switch (mParentDevice->deviceModel())
        {
        case DeviceModel::DES3526:
            portInfo = new SwitchPortDes3526(this);
            break;
        case DeviceModel::DES3528:
            portInfo = new SwitchPortDes3528(this);
            break;
        case DeviceModel::DES3550:
            portInfo = new SwitchPortDes3550(this);
            break;
        default:
            return;
        }

        portInfo->setIndex(i);

        mList.push_back(portInfo);
    }
}

bool SwitchPortTableModel::getUnicastVlanSettings(const OidPair &oidVlan,
                                                  QBitArray &vlanPortArray,
                                                  QString vlanName)
{
    QScopedPointer<SnmpClient> snmp(new SnmpClient());

    snmp->setIp(mParentDevice->ip());

    if (!snmp->setupSession(SessionType::ReadSession)) {
        mError = SnmpErrorStrings::SetupSession;

        return false;
    }

    if (!snmp->openSession()) {
        mError = SnmpErrorStrings::OpenSession;

        return false;
    }

    snmp->createPdu(SNMP_MSG_GET);

    snmp->addOid(oidVlan);

    bool result;

    if (snmp->sendRequest()) {
        netsnmp_variable_list *vars = snmp->varList();

        if ((vars->val_len < 4) ||
                (snmp_oid_ncompare(oidVlan.first, oidVlan.second,
                                   vars->name, vars->name_length,
                                   oidVlan.second) != 0)) {
            mError = QString::fromUtf8("Ошибка: Не удалось получить данные по влану %1.")
                     .arg(vlanName);

            return false;
        }

        vlanPortArray = ucharToQBitArray(mParentDevice->deviceModel(),
                                         vars->val.string);

        result = true;
    } else {
        mError = QString::fromUtf8("Ошибка: Не удалось получить данные по влану %1.")
                 .arg(vlanName);
        result = false;
    }

    return result;
}

bool SwitchPortTableModel::getMulticastVlanSettings()
{
    QScopedPointer<SnmpClient> snmpClient(new SnmpClient());

    snmpClient->setIp(mParentDevice->ip());

    if (!snmpClient->setupSession(SessionType::ReadSession)) {
        mError = SnmpErrorStrings::SetupSession;
        return false;
    }

    if (!snmpClient->openSession()) {
        mError = SnmpErrorStrings::SetupSession;
        return false;
    }

    snmpClient->createPdu(SNMP_MSG_GETNEXT);

    OidPair mvrMemberOid;
    if (mParentDevice->deviceModel() == DeviceModel::DES3526) {
        mvrMemberOid = createOidPair(Mib::swL2IGMPMulticastVlanMemberPortDES3526, 15);
    } else if (mParentDevice->deviceModel() == DeviceModel::DES3528) {
        mvrMemberOid = createOidPair(Mib::swL2IGMPMulticastVlanMemberPortDES3528, 13);
    } else if (mParentDevice->deviceModel() == DeviceModel::DES3550) {
        mvrMemberOid = createOidPair(Mib::swL2IGMPMulticastVlanMemberPortDES3550, 15);
    } else {
        return false;
    }

    snmpClient->addOid(mvrMemberOid);

    if (snmpClient->sendRequest()) {
        netsnmp_variable_list *vars = snmpClient->varList();

        if ((vars->val_len < 4) ||
                (snmp_oid_ncompare(mvrMemberOid.first, mvrMemberOid.second,
                                   vars->name, vars->name_length,
                                   mvrMemberOid.second) != 0)) {
            mError = QString::fromUtf8("Ошибка: Не удалось получить данные по Multicast влану.");

            return false;
        }

        mIptvMultVlanTag = vars->name[vars->name_length - 1];
        mMulticastVlanMember = ucharToQBitArray(mParentDevice->deviceModel(), vars->val.string);

        return true;
    } else {
        mError = QString::fromUtf8("Ошибка: Не удалось получить данные по Multicast влану.");

        return false;
    }
}

bool SwitchPortTableModel::sendVlanSetting(QVector<OidPair> &oidList,
                                           QList<QBitArray> &arrayList, bool ismv)
{
    QScopedPointer<SnmpClient> snmpClient(new SnmpClient());

    snmpClient->setIp(mParentDevice->ip());

    if (!snmpClient->setupSession(SessionType::WriteSession)) {
        mError = SnmpErrorStrings::SetupSession;
        return false;
    }

    if (!snmpClient->openSession()) {
        mError = SnmpErrorStrings::OpenSession;
        return false;
    }

    snmpClient->createPdu(SNMP_MSG_SET);

    int size = arrayList.size();
    for (int i = 0; i < size; ++i) {
        QString arrayString = qbitArrayToHexString(mParentDevice->deviceModel(),
                                                   arrayList[i], ismv);
        snmpClient->addOid(oidList.at(i), arrayString, 'x');
    }

    bool result = true;

    if (!snmpClient->sendRequest()) {
        mError = QString::fromUtf8("Ошибка: Не удалось изменить настройки порта!");
        result = false;
    }

    return result;
}

QBitArray SwitchPortTableModel::ucharToQBitArray(DeviceModel::Enum deviceModel, uchar *str)
{
    int countBits = 0;

    if ((deviceModel == DeviceModel::DES3526)
            || (deviceModel == DeviceModel::DES3528)) {
        countBits = 32;
    } else if (deviceModel == DeviceModel::DES3550) {
        countBits = 56;
    }

    QBitArray array(countBits, false);

    int firstBit = 0;
    int countBytes = countBits / 8;

    for (int i = 0; i < countBytes; ++i) {
        quint8 byteElement = str[i];

        if (byteElement == 0) {
            //так как массив уже инициализирован нулями
            //если текущий байт равен 0, итерация просто пропускается
            firstBit += 8;
            continue;
        }

        for (int bits = 0; bits < 8; ++bits) {
            array[7 - bits + firstBit] = byteElement % 2 == 1 ? true : false;

            byteElement /= 2;
        }

        firstBit += 8;
    }

    return array;
}

QString SwitchPortTableModel::qbitArrayToHexString(DeviceModel::Enum deviceModel, const QBitArray &array, bool ismv)
{
    QString tempStr = "";

    quint8 value = 0;

    int size = array.size();
    for (int i = 0; i < size; ++i) {
        value <<= 1;
        value += (quint8)array.at(i);

        if ((i + 1) % 8 == 0) { //проверка на последний bit в текущем байте
            tempStr.push_back(QString(" %1").arg(value, 2, 16, QLatin1Char('0')));
            value = 0;
        }
    }

    if ((deviceModel == DeviceModel::DES3528) && ismv) {
        // Значение MembersISMV в DES-3528 имеет размер 64 байта
        // Дополняем HEX-строку до 64 байт.
        for (int i = 0; i < 60; ++i)
            tempStr.push_back(QString(" 00"));
    }

    return tempStr.trimmed().toUpper();
}
