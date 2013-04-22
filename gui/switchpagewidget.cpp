#include "switchpagewidget.h"
#include "ui_switchpagewidget.h"

#include <basicdialogs.h>
#include <constant.h>
#include <devices/switch.h>
#include <models/mactablemodel.h>
#include <models/switchporttablemodel.h>

SwitchPageWidget::SwitchPageWidget(Device::Ptr deviceInfo, QWidget *parent) :
    PageWidget(deviceInfo, parent),
    ui(new Ui::SwitchPageWidget)
{
    ui->setupUi(this);

    ui->portInfoGroupBox->setVisible(false);
    ui->portInfoGroupBox->setChecked(false);

    connect(ui->portListTableView, &QTableView::customContextMenuRequested,
            this, &SwitchPageWidget::portViewRequestContextMenu);
    connect(ui->macAddressTableView, &QTableView::customContextMenuRequested,
            this, &SwitchPageWidget::macTableViewRequestContextMenu);
    connect(ui->portInfoGroupBox, &QGroupBox::toggled,
            ui->portInfoGroupBox, &QGroupBox::setVisible);

    connect(ui->macRadioButton, &QRadioButton::toggled,
            this, &SwitchPageWidget::macRadioButtonChangeState);
    connect(ui->macLineEdit, &QLineEdit::textChanged,
            this, &SwitchPageWidget::macLineEditTextChanged);
    connect(ui->filterPortButton, &QPushButton::pressed,
            this, &SwitchPageWidget::filterMacAddressByPorts);

    connect(ui->showPortInfoAction, &QAction::triggered,
            this, &SwitchPageWidget::showPortInfoGroupBox);
    connect(ui->refreshPortInfoAction, &QAction::triggered,
            this, &SwitchPageWidget::refreshPortInfo);
    connect(ui->refreshAllPortInfoAction, &QAction::triggered,
            this, &SwitchPageWidget::refreshAllPortInfo);
    connect(ui->refreshMacTableInfoAction, &QAction::triggered,
            this, &SwitchPageWidget::refreshMacTable);
    connect(ui->addPortToMulticastVlanAction, &QAction::triggered,
            this, &SwitchPageWidget::addPortToMulticastVlan);
    connect(ui->removePortFromMulticastVlanAction, &QAction::triggered,
            this, &SwitchPageWidget::removePortFromMulticastVlan);
    connect(ui->setPortInetServiceAction, &QAction::triggered,
            this, &SwitchPageWidget::setPortInternetService);
    connect(ui->setPortInetWithIptvStbServiceAction, &QAction::triggered,
            this, &SwitchPageWidget::setPortInternetWithStbService);

    //Инициализация модели списка портов
    Switch::Ptr switchInfo = mDeviceInfo.objectCast<Switch>();

    SwitchPortTableModel *portListModel = new SwitchPortTableModel(switchInfo, this);

    portListModel->updateInfoAllPort();

    ui->portListTableView->setModel(portListModel);
    ui->portListTableView->setColumnWidth(0, 50);
    ui->portListTableView->setColumnWidth(1, 70);
    ui->portListTableView->setColumnWidth(2, 145);

    //Инициализация модели таблицы mac-адресов
    MacTableModel *macListModel = new MacTableModel(switchInfo, this);

    if (!macListModel->update())
        BasicDialogs::error(this, BasicDialogStrings::Error, macListModel->error());

    QSortFilterProxyModel *macListFilterProxyModel = new QSortFilterProxyModel(this);
    macListFilterProxyModel->setFilterRole(Qt::DisplayRole);
    macListFilterProxyModel->setFilterKeyColumn(2);
    macListFilterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    macListFilterProxyModel->setSourceModel(macListModel);

    ui->macAddressTableView->setModel(macListFilterProxyModel);
    ui->macAddressTableView->setColumnWidth(0, 70);
    ui->macAddressTableView->setColumnWidth(1, 100);

    if (switchInfo->deviceModel() == DeviceModel::DES3550) {
        ui->portListLineEdit->setText("1-50");
    } else {
        ui->portListLineEdit->setText("1-26");
    }
}

SwitchPageWidget::~SwitchPageWidget()
{
    delete ui;
}

void SwitchPageWidget::saveSwitchConfig()
{
    bool result = mDeviceInfo.objectCast<Switch>()->saveConfig();

    if (result) {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Запрос на сохранение конфигурации коммутатора отправлен."));
    } else {
        BasicDialogs::information(this, BasicDialogStrings::Error, mDeviceInfo->error());
    }
}

void SwitchPageWidget::refreshPortInfo()
{
    SwitchPortTableModel *portListModel = static_cast<SwitchPortTableModel *>(ui->portListTableView->model());

    QModelIndex index = ui->portListTableView->currentIndex();
    QModelIndex portIndex = portListModel->index(index.row(), 0);
    int port = portListModel->data(portIndex).toInt();

    if (!index.isValid())
        return;

    bool result = portListModel->updateInfoPort(port);

    if (!result) {
        BasicDialogs::information(this, BasicDialogStrings::Error, portListModel->error());
    } else {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Информация по порту %1 обновлена.").arg(port));

        if (ui->portInfoGroupBox->isVisible())
            showPortInfoGroupBox();
    }
}

void SwitchPageWidget::refreshAllPortInfo()
{
    SwitchPortTableModel *portListModel = static_cast<SwitchPortTableModel *>(ui->portListTableView->model());

    ui->portInfoGroupBox->setChecked(false);

    if (!portListModel->updateInfoAllPort()) {
        BasicDialogs::error(this, BasicDialogStrings::Error, portListModel->error());
    } else {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Информация по всем портам обновлена."));
    }
}

void SwitchPageWidget::refreshMacTable()
{
    QSortFilterProxyModel *proxyModel = static_cast<QSortFilterProxyModel *>(ui->macAddressTableView->model());
    MacTableModel *macListModel = static_cast<MacTableModel *>(proxyModel->sourceModel());

    if (!macListModel->update()) {
        BasicDialogs::error(this, BasicDialogStrings::Error, macListModel->error());
    } else {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Информация в таблице MAC-адресов обновлена."));
    }
}

void SwitchPageWidget::addPortToMulticastVlan()
{
    changeStateSwitchPortInMulticastVlan(true);
}

void SwitchPageWidget::removePortFromMulticastVlan()
{
    changeStateSwitchPortInMulticastVlan(false);
}

void SwitchPageWidget::changeStateSwitchPortInMulticastVlan(bool state)
{
    SwitchPortTableModel *portListModel = static_cast<SwitchPortTableModel *>(ui->portListTableView->model());

    QModelIndex index = ui->portListTableView->currentIndex();
    QModelIndex portIndex = portListModel->index(index.row(), 0);
    int port = portListModel->data(portIndex).toInt();

    if (!index.isValid())
        return;

    if (port == 25 || port == 26) {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Настройки на данном порту менять нельзя."));
        return;
    }

    bool result = portListModel->setMemberMulticastVlan(port, state);

    if (!result) {
        BasicDialogs::error(this, BasicDialogStrings::Error, portListModel->error());
    } else {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Настойки порта изменены."));

        if (ui->portInfoGroupBox->isVisible())
            showPortInfoGroupBox();
    }
}

void SwitchPageWidget::setPortInternetService()
{
    SwitchPortTableModel *portListModel = static_cast<SwitchPortTableModel *>(ui->portListTableView->model());

    QModelIndex index = ui->portListTableView->currentIndex();
    QModelIndex portIndex = portListModel->index(index.row(), 0);
    int port = portListModel->data(portIndex).toInt();

    if (!index.isValid())
        return;

    if (port == 25 || port == 26) {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Настройки на данном порту менять нельзя."));
        return;
    }

    bool result = portListModel->setMemberInternetService(port);

    if (!result) {
        BasicDialogs::error(this, BasicDialogStrings::Error, portListModel->error());
    } else {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Порт %1 настроен для Интернета.").arg(port));

        if (ui->portInfoGroupBox->isVisible())
            showPortInfoGroupBox();
    }
}

void SwitchPageWidget::setPortInternetWithStbService()
{
    SwitchPortTableModel *portListModel = static_cast<SwitchPortTableModel *>(ui->portListTableView->model());

    QModelIndex index = ui->portListTableView->currentIndex();
    QModelIndex portIndex = portListModel->index(index.row(), 0);
    int port = portListModel->data(portIndex).toInt();

    if (!index.isValid())
        return;

    if (port == 25 || port == 26) {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Настройки на данном порту менять нельзя."));
        return;
    }

    bool result = portListModel->setMemberInternetWithIptvStbService(port);

    if (!result) {
        BasicDialogs::error(this, BasicDialogStrings::Error, portListModel->error());
    } else {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Порт настроен для Интернета + Iptv Stb"));

        if (ui->portInfoGroupBox->isVisible())
            showPortInfoGroupBox();
    }
}

void SwitchPageWidget::filterMacAddressByPorts()
{
    QSortFilterProxyModel *proxyModel = static_cast<QSortFilterProxyModel *>(ui->macAddressTableView->model());

    proxyModel->setFilterKeyColumn(0);

    if (ui->portListLineEdit->text().isEmpty()) {
        BasicDialogs::information(this, BasicDialogStrings::Info, QString::fromUtf8("Перед фильтрованием необходимо ввести список портов."));
        return;
    }

    QString regExpStr = "";

    QStringList rangeStringList = ui->portListLineEdit->text().split(QRegExp(","), QString::SkipEmptyParts);

    auto it = rangeStringList.constBegin();
    auto end = rangeStringList.constEnd();
    for (; it != end; ++it) {
        QStringList valueStringList = (*it).split("-", QString::SkipEmptyParts);

        if (valueStringList.size() == 2) {
            int first = valueStringList.at(0).toInt();
            int last = valueStringList.at(1).toInt();

            for (int i = first; i <= last; ++i)
                regExpStr += QString("^%1$|").arg(i);
        } else {
            regExpStr += QString("^%1$|").arg(valueStringList.at(0));
        }
    }

    if (regExpStr[regExpStr.length() - 1] == '|')
        regExpStr.remove(regExpStr.length() - 1, 1);

    QRegExp regExp(regExpStr, Qt::CaseInsensitive, QRegExp::RegExp);
    proxyModel->setFilterRegExp(regExp);
}

void SwitchPageWidget::macLineEditTextChanged(QString text)
{
    QSortFilterProxyModel *proxyModel = static_cast<QSortFilterProxyModel *>(ui->macAddressTableView->model());

    text = ui->macLineEdit->displayText();
    text.replace('_', "?");

    proxyModel->setFilterKeyColumn(2);
    proxyModel->setFilterWildcard(text);
}

void SwitchPageWidget::macRadioButtonChangeState(bool checked)
{
    ui->macLineEdit->setEnabled(checked);
    ui->portListLineEdit->setEnabled(!checked);
    ui->filterPortButton->setEnabled(!checked);

    if (checked) {
        macLineEditTextChanged(ui->macLineEdit->text());
    } else {
        filterMacAddressByPorts();
    }
}

void SwitchPageWidget::showPortInfoGroupBox()
{
    SwitchPortTableModel *portListModel = static_cast<SwitchPortTableModel *>(ui->portListTableView->model());

    QModelIndex index = ui->portListTableView->currentIndex();

    if (!index.isValid())
        return;

    QModelIndex portIndex = portListModel->index(index.row(), 0);
    int port = portListModel->data(portIndex).toInt();
    ui->portInetVlanValueLabel->setText(VlanState::toString(portListModel->memberInetVlan(port)));
    ui->portIptvVlanValueLabel->setText(VlanState::toString(portListModel->memberIptvVlan(port)));

    if (portListModel->memberMulticastVlan(port)) {
        ui->multicastVlanValueLabel->setPixmap(QPixmap(QString::fromUtf8(":/images/yes.png")));
    } else {
        ui->multicastVlanValueLabel->setPixmap(QPixmap(QString::fromUtf8(":/images/no.png")));
    }

    ui->portInfoGroupBox->setTitle(QString::fromUtf8("Порт %1").arg(port));
    ui->portInfoGroupBox->setChecked(true);
}

void SwitchPageWidget::portViewRequestContextMenu(QPoint point)
{
    QMenu contextMenu(this);
    contextMenu.addAction(ui->showPortInfoAction);
    contextMenu.addAction(ui->refreshPortInfoAction);
    contextMenu.addSeparator();

    QMenu *mvlanMenu = contextMenu.addMenu(QIcon(":/images/tv.png"), "Multicast Vlan");
    mvlanMenu->addAction(ui->addPortToMulticastVlanAction);
    mvlanMenu->addAction(ui->removePortFromMulticastVlanAction);

    QMenu *settingsPortMenu = contextMenu.addMenu(QString::fromUtf8("Установить сервис"));
    settingsPortMenu->addAction(ui->setPortInetServiceAction);
    settingsPortMenu->addAction(ui->setPortInetWithIptvStbServiceAction);

    contextMenu.addSeparator();
    contextMenu.addAction(ui->refreshAllPortInfoAction);

    contextMenu.exec(ui->portListTableView->mapToGlobal(point));
}

void SwitchPageWidget::macTableViewRequestContextMenu(QPoint point)
{
    QMenu contextMenu(this);
    contextMenu.addAction(ui->refreshMacTableInfoAction);

    contextMenu.exec(ui->macAddressTableView->mapToGlobal(point));
}