#include "q_rename.h"
#include "ui_q_rename.h"

q_rename::q_rename(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::q_rename)
{
    m_ui->setupUi(this);
}

std::tuple<int, QString> q_rename::exec(const QString& name)
{
    //set text
    m_ui->q_le_name->setText(name);
    //exec
    return std::tuple<int, QString> ( QDialog::exec(), m_ui->q_le_name->text() );
}

q_rename::~q_rename()
{
    delete m_ui;
}
