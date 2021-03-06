#ifndef SWITCHPORTTABLEDELEGATE_H
#define SWITCHPORTTABLEDELEGATE_H

#include <stdafx.h>

class SwitchPortTableDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit SwitchPortTableDelegate(int descriptionPortLength = 0, QObject *parent = 0);
    
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private:
    int mDescriptionPortLength;
};

#endif // SWITCHPORTTABLEDELEGATE_H
