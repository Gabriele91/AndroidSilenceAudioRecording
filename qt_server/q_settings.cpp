#include "q_settings.h"
#include "ui_q_settings.h"
#include <q_android_silence_audio_recording.h>
#include <q_audio_server_listener.h>
#include <QString>
q_settings::q_settings(q_android_silence_audio_recording *parent)
:QWidget(parent)
,m_asar(parent)
,m_ui(new Ui::q_settings)
{
    m_ui->setupUi(this);
}

void q_settings::set_audio_server_listener(q_audio_server_listener* listener)
{
    //save listener
    m_listener = listener;
    //build string
    m_ui->m_lb_title->setText(
    build_item_string(QString::fromUtf8(m_listener->get_android_id().c_str()),
                      QString::fromUtf8(m_listener->get_imei().c_str()),
                      m_listener->connected())
    );
    //enable setting and disable player
    m_ui->m_gb_settings->setEnabled(true);
    m_ui->m_gb_player->setEnabled(false);
}

q_settings::~q_settings()
{
    delete m_ui;
}


void q_settings::back_to_device_list()
{
    stop();
    cleanup_info();
    m_asar->show_list_device();
}

void q_settings::cleanup_info()
{
    m_ui->m_le_path->setText("");
    m_ui->m_cb_channels->setCurrentIndex(0);
    m_ui->m_cb_samples->setCurrentIndex(0);
    m_ui->m_cb_play_stop->setText("PLAY");
    m_ui->m_cb_play_stop->setChecked(false);
    m_ui->m_gb_settings->setEnabled(true);
    m_ui->m_gb_player->setEnabled(false);
}

void q_settings::applay_settings()
{
   if
   (  m_listener->connected() &&
     (m_listener->state()==S_CONN ||
      m_listener->state()==S_INFO ||
      m_listener->state()==S_STOP)
   )
   {
       unsigned int channels = m_ui->m_cb_channels->currentText().toInt();
       unsigned int samples  = m_ui->m_cb_samples->currentText().toInt();
       m_listener->init({
                            channels,
                            samples,
                            16
                        });
       m_listener->send_meta_info(m_asar->get_rak_server());
       m_ui->m_cb_play_stop->setText("PLAY");
       m_ui->m_cb_play_stop->setChecked(false);
       m_ui->m_gb_settings->setEnabled(false);
       m_ui->m_gb_player->setEnabled(true);
   }
}

void q_settings::select_path()
{

}

void q_settings::save()
{

}

void q_settings::play_or_pause()
{
    if(  m_listener->state()==S_INFO ||
         m_listener->state()==S_STOP ||
         m_listener->state()==S_PAUSE )
    {
        m_listener->send_start(m_asar->get_rak_server());
        m_listener->output_play();
        m_ui->m_cb_play_stop->setText("PAUSE");
    }
    else
    {
        if ( m_listener->state()==S_REC )
        {
            m_listener->send_pause(m_asar->get_rak_server());
        }
        //in any case
        m_ui->m_cb_play_stop->setText("PLAY");
        m_ui->m_cb_play_stop->setChecked(false);
    }
}

void q_settings::stop()
{

    if
    (  m_listener->connected() &&
      (m_listener->state()==S_REC ||
       m_listener->state()==S_PAUSE) )
    {
        m_listener->send_stop(m_asar->get_rak_server());
        m_listener->output_stop();
        m_ui->m_cb_play_stop->setText("PLAY");
        m_ui->m_cb_play_stop->setChecked(false);
    }
}
