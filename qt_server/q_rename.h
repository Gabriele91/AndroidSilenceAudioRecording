#ifndef Q_RENAME_H
#define Q_RENAME_H

#include <tuple>
#include <QDialog>

namespace Ui {
class q_rename;
}

class q_rename : public QDialog
{
    Q_OBJECT

public:
    explicit q_rename(QWidget *parent = 0);
    ~q_rename();
    std::tuple<int, QString> exec(const QString& name);

private:
    Ui::q_rename *m_ui;
};

#endif // Q_RENAME_H
