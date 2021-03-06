#ifndef STDAFX_H
#define STDAFX_H

#if (defined(_WIN32) || defined(__WIN32__))
#include <winsock2.h>
#endif

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/version.h>

#include <QtCore/QBitArray>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QModelIndex>
#include <QtCore/QPoint>
#include <QtCore/QRegExp>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStandardPaths>
#include <QtCore/QStringBuilder>
#include <QtCore/QStringListModel>
#include <QtCore/QTranslator>
#include <QtCore/QVector>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

#include <QtConcurrent/QtConcurrent>

#include <QtGui/QBrush>
#include <QtGui/QCloseEvent>
#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QValidator>

#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>

#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSplashScreen>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QToolButton>

#endif // STDAFX_H
