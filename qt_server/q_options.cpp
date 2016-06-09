#include "q_options.h"
#include "ui_q_options.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#define MAX_CLIENTS 255
#define SERVER_PORT 8000

q_options::q_options(QWidget *parent)
:QDialog(parent)
,m_ui(new Ui::q_options)
,m_settings(QSettings::IniFormat, QSettings::UserScope,
          "Unipg",
          "AndroidSilenceAudioRecording")
{
    //init ui
    m_ui->setupUi(this);
    //default path
    m_last_path = QDir::homePath();
    //can't mod path string
    m_ui->m_le_path->setEnabled(false);
    //set last path
    m_ui->m_le_path->setText(get_path());
    //set last port
    m_ui->m_sb_port->setValue(get_port());
    //default rejected
    m_close_state = QMessageBox::Cancel;
}

int q_options::exec()
{
    //default rejected
    m_close_state = QMessageBox::Cancel;
    //set last path
    m_ui->m_le_path->setText(get_path());
    //set last port
    m_ui->m_sb_port->setValue(get_port());
    //exec
    QDialog::exec();
    //return
    return m_close_state;
}


void q_options::accepted()
{
    m_close_state = QMessageBox::Ok;
    //save
    {
        m_settings.setValue("PATH",m_ui->m_le_path->text());
        m_settings.setValue("PORT",m_ui->m_sb_port->value());
    }
    close();
}

void q_options::rejected()
{
    m_close_state = QMessageBox::Cancel;
    close();
}

void q_options::set_path()
{
    QString default_path =
    QFileDialog::getExistingDirectory(this,
                                      tr("Select files save path"),
                                      m_last_path,
                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    //valid path?
    if(default_path != QString::null)
    {

        if(QFileInfo(default_path).isWritable())
        {
            m_ui->m_le_path->setText(default_path);
        }
        else
        {
            QMessageBox::warning(
                this,
                tr("AndroidSilenceAudioRecording"),
                tr("I can't access this directory.")
            );
        }
    }
}

void q_options::set_port(int port)
{
    //none
}

QString q_options::get_path() const
{
    if(m_settings.contains("PATH"))
    {
        return m_settings.value("PATH").toString();
    }
    else
    {
        return QDir::homePath();
    }
}

unsigned short q_options::get_port() const
{
    if(m_settings.contains("PORT"))
    {
        return (unsigned short)m_settings.value("PORT").toInt();
    }
    else
    {
        return SERVER_PORT;
    }
}


unsigned short q_options::get_max_clients() const
{
    return MAX_CLIENTS;
}


q_options::~q_options()
{
    delete m_ui;
}
