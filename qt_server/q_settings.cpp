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
    //setup ui
    m_ui->setupUi(this);
    //setting plotte
    m_ui->m_plotter->setBackground(QColor{"#302F2F"});
    m_ui->m_plotter->addGraph();
    m_ui->m_plotter->xAxis->setAutoTickLabels(false);
    m_ui->m_plotter->yAxis->setAutoTickLabels(false);
    m_ui->m_plotter->xAxis->setTickLabels(false);
    m_ui->m_plotter->yAxis->setTickLabels(false);
    m_ui->m_plotter->xAxis->setVisible(false);
    m_ui->m_plotter->yAxis->setVisible(false);
    m_ui->m_plotter->yAxis->setRange(-150,150);
    m_ui->m_plotter->axisRect()->setAutoMargins(QCP::msNone);
    m_ui->m_plotter->axisRect()->setMargins(QMargins(0,0,0,0));
    //set default pen
    QPen pen;
    pen.setColor(QColor(38, 38, 255, 255));
    pen.setWidthF(2);
    m_ui->m_plotter->graph(0)->setPen(pen);
    //default path
    m_last_path = QDir::homePath();
    //applay update
    m_timer = new QBasicTimer();

    ////////////////////////////////////////////////////////////////////////
    //size of plotter
    m_ui->m_plotter->xAxis->setRange(0,9);
    //void plotter
    m_x_values.resize(10);
    m_y_values.resize(10);
    //init x
    for(int i=0;i!=m_x_values.size();++i) m_x_values[i] = (double)i;
    //init y
    m_y_values.fill(0.0);
    m_ui->m_plotter->graph(0)->setData(m_x_values,m_y_values);
    m_ui->m_plotter->replot();
    ////////////////////////////////////////////////////////////////////////
}
q_settings::~q_settings()
{
    m_timer->stop();
    delete m_timer;
    delete m_ui;
}

void q_settings::set_audio_server_listener(q_audio_server_listener* listener)
{
    //save listener
    m_listener = listener;
    //reset
    cleanup_info();
    //volume to max
    m_listener->output_set_volume(1.0);
    //build string callback
    auto callback =
    [this](bool connected)
    {
        //no connected? Reset
        if(!connected) cleanup_info();
        //rebuild string
        QString android_id  = QString::fromUtf8(m_listener->get_android_id().c_str());
        QString android_imei= QString::fromUtf8(m_listener->get_imei().c_str());
        m_ui->m_lb_title->setText(build_item_string(android_id, android_imei, connected));
    };
    //first call
    callback(m_listener->connected());
    //change state
    m_listener->change_connession_callback(callback);
}



void q_settings::timerEvent(QTimerEvent *etime)
{
    if(m_listener && m_listener->state() == S_REC)
    {
        append_sample(m_listener->output_value());
        m_ui->m_plotter->replot();
    }
}

void q_settings::back_to_device_list()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Return to device liste",
                                  "The recording will be discarded, you are sure?",
                                  QMessageBox::Yes | QMessageBox::Cancel,
                                  QMessageBox::Cancel);
    //dialog
    if (reply == QMessageBox::Yes)
    {
        //stop rec
        stop();
        //remove reference
        m_listener->change_connession_callback(nullptr);
        //clear file
        m_listener->clear_buffer_file();
        //clean info
        cleanup_info();
        //go back
        m_asar->show_list_device();
    }
}

void q_settings::cleanup_info()
{
    m_ui->m_hs_sound->setValue(m_ui->m_hs_sound->maximum());
    m_ui->m_cb_samples->setCurrentIndex(0);
    m_ui->m_cb_play_stop->setText("PLAY");
    m_ui->m_cb_play_stop->setChecked(false);
    m_ui->m_gb_settings->setEnabled(true);
    m_ui->m_gb_player->setEnabled(false);
    //reset plotter
    m_y_values.fill(0);
    m_ui->m_plotter->graph(0)->setData(m_x_values,m_y_values);
    m_ui->m_plotter->replot();
}


static double lerp(double left,double right,double l)
{
    return left*l+(right*(1.0-l));
}

void q_settings::append_sample(short sample)
{
    if(!m_y_values.size()) return;
    //gest last
    double last = m_y_values.last();
    //put all value shifted by 1
    for(int j=1;j < m_y_values.size();++j)
    {
        m_y_values[j-1] = m_y_values[j];
    }
    //interpolation factor
    const double interpolation = 0.60;
    //put value
    m_y_values.last() = lerp(last,(double) sample,interpolation);
    //add vectors
    m_ui->m_plotter->graph(0)->setData(m_x_values,m_y_values);
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
       const unsigned int channels = 1;
       const unsigned int samples  = m_ui->m_cb_samples->currentText().toInt();
       m_listener->init({
                            channels,
                            samples*channels,
                            16
                        });
       //init timer
       const int ms_update = 5;
       m_timer->stop();
       m_timer->start(ms_update,this);
       //alloc
       m_x_values.resize(samples*channels/ms_update);
       m_y_values.resize(samples*channels/ms_update);
       //size of plotter
       m_ui->m_plotter->xAxis->setRange(0,samples*channels/ms_update-1);
       //init x
       for(int i=0;i!=m_x_values.size();++i) m_x_values[i] = (double)i;
       //init y
       m_y_values.fill(0.0);
       //applay
       m_listener->send_meta_info(m_asar->get_rak_server());
       m_ui->m_cb_play_stop->setText("PLAY");
       m_ui->m_cb_play_stop->setChecked(false);
       m_ui->m_gb_settings->setEnabled(false);
       m_ui->m_gb_player->setEnabled(true);
   }
}

void q_settings::save()
{
    if(m_listener)
    {
        QString filename = QFileDialog::getSaveFileName
                (  this,
                   tr("Save recording"),
                   m_last_path,
                   tr("Waveform Audio File Format (*.wav)")
                );

        if(!m_listener->save_file(filename.toUtf8().constData()) && filename!=QString::null )
        {
            QMessageBox::about(this,"Abort","Fail to save file.\nCan't save : "+filename);
        }
        else
        {
            //save last valid path
            m_last_path = filename;
        }
    }
}

void q_settings::volume(int value)
{
    if(m_listener)
    {
        m_listener->output_set_volume((double(m_ui->m_hs_sound->value()))/100.0);
    }
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
            m_listener->output_stop();
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
