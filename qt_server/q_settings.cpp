#include "q_settings.h"
#include "ui_q_settings.h"
#include <q_android_silence_audio_recording.h>
#include <q_audio_server_listener.h>
#include <q_audio_player.h>
#include <q_rename.h>
#include <QDateTime>
#include <QString>
#include <string>
q_settings::q_settings(q_android_silence_audio_recording *parent)
:QWidget(parent)
,m_asar(parent)
,m_ui(new Ui::q_settings)
,m_player(new q_audio_player)
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
    delete m_player;
    delete m_timer;
    delete m_ui;
}

void q_settings::set_audio_server_listener(q_audio_server_listener* listener, const QString &path)
{
    //save listener
    m_listener = listener;
    //save path
    m_dest_path = path;
    //reset
    cleanup_info();
    //create name
    QString str_base_android_id =  tr(listener->get_android_id().c_str());
    QString str_android_id      =  tr("id_") + str_base_android_id + tr("_") ;
    QString str_datetime        =  QDateTime().currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    //create default name
    m_default_output_name = str_android_id + str_datetime ;
    //set to textbox
    m_ui->m_lb_name->setText(m_default_output_name + ".wav");
    //volume to max
    m_player->set_volume(1.0);
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
        append_sample(m_player->output_value());
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
    m_ui->m_hl_to_apply->setEnabled(true);
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


void q_settings::apply_settings()
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
       //create info
       input_meta_info meta_info =
       {
           channels,
           samples*channels,
           16
       };
       //init listener
       m_listener->init(meta_info);
       //init audio output
       m_player->init(meta_info);
       m_player->stop();
       //attach
       m_listener->set_output_buffer(m_player->get_buffer_output());
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
       m_ui->m_hl_to_apply->setEnabled(false);
       m_ui->m_gb_player->setEnabled(true);
       /////////////////////////////////////////////////////////////////////////////////
       /////////////////////////////////////////////////////////////////////////////////
       //openfile
       //get data
       QDate this_date = QDateTime::currentDateTime().date();
       //build string
       std::string date_str =       std::to_string(this_date.year())
                               +"-"+std::to_string(this_date.month())
                               +"-"+std::to_string(this_date.day());
       //path file
       QString file_path_name = m_dest_path+"/"+m_ui->m_lb_name->text();
       //only file name
       QString q_file_name        = QFileInfo(file_path_name).baseName();
       std::string only_file_name = q_file_name.toStdString();
       //INFO META
       wav_riff::info_fields fields_riff_meta_info =
       {
           {  {'I','N','A','M'}, only_file_name                        },
           {  {'I','A','R','T'}, "Android Silence Audio Recording, "
                                 +m_listener->get_android_id()
           },
           {  {'I','C','O','P'}, "Gabriele Di Bari, Giulio Biondi"     },
           {  {'I','C','M','T'},
               "File generated with AndroidSilenceAudioRecording.\n"
               "Android id: "+m_listener->get_android_id() +"\n"
               "IMEI id: "   +m_listener->get_imei() +"\n"
               "date: "      +date_str
           },
           {  {'I','C','R','D'}, date_str }
       };
       //init file output
       m_listener->open_output_file(file_path_name.toStdString(),
                                    fields_riff_meta_info);
       /////////////////////////////////////////////////////////////////////////////////
       /////////////////////////////////////////////////////////////////////////////////

   }
}

static bool create_file_md5(const QString &file_path)
{
   QFile i_file(file_path);
   QFile o_file(file_path+".md5");
   QCryptographicHash hash(QCryptographicHash::Md5);
   QByteArray hash_res;

   if (i_file.open(QFile::ReadOnly) &&
       o_file.open(QFile::WriteOnly) &&
       hash.addData(&i_file))
   {
       //get res
       hash_res = hash.result();
       //write to file
       o_file.write(hash_res.toHex());
       //ok
       return true;
   }
   return false;
}

void q_settings::rename()
{
    if(m_listener)
    {
        //alloc and init
        q_rename dialog_rename;
        //exect
        auto result = dialog_rename.exec(m_default_output_name) ;
        //exe
        if(std::get<0>(result)== QDialog::Accepted)
        {
            //get
            QString new_name = std::get<1>(result);
            //new path
            QString dest = m_dest_path+"/"+new_name+".wav";
            //test
            if(!new_name.length())
            {
                QMessageBox dialog_err;
                dialog_err.critical(0,"Error","File name not valid!");
                dialog_err.setFixedSize(500,200);
                return;
            }
            else if(QFileInfo(dest).exists())
            {
                QMessageBox dialog_err;
                dialog_err.critical(0,"Error","File already exists!");
                dialog_err.setFixedSize(500,200);
                return;
            }
            //save new name
            m_default_output_name = new_name;
            //set to textbox
            m_ui->m_lb_name->setText(m_default_output_name + ".wav");
        }
    }
}

void q_settings::volume(int value)
{
    m_player->set_volume((double(m_ui->m_hs_sound->value()))/100.0);
}

void q_settings::play_or_pause()
{
    if(  m_listener->state()==S_INFO ||
         m_listener->state()==S_STOP ||
         m_listener->state()==S_PAUSE )
    {
        m_listener->send_start(m_asar->get_rak_server());
        m_player->play();
        m_ui->m_cb_play_stop->setText("PAUSE");
    }
    else
    {
        if ( m_listener->state()==S_REC )
        {
            m_listener->send_pause(m_asar->get_rak_server());
            m_player->stop();
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
        //send stop
        m_listener->send_stop(m_asar->get_rak_server());
        //save file
        close_file();
        //stop draw plot
        m_player->stop();
        //reset ui
        m_ui->m_cb_play_stop->setText("PLAY");
        m_ui->m_cb_play_stop->setChecked(false);
    }
}

void q_settings::close_file()
{
    //path
    auto& file_path_name = m_listener->get_output_path();
    //try to save
    if(m_listener->close_output_file())
    {
        //create md5 file
        if(!create_file_md5(file_path_name))
        {
            QMessageBox::about(this,"Abort","Fail to create hash file.\nCan't save : "+file_path_name+".md5");
        }
    }
    else
    {
        QMessageBox::about(this,"Abort","Fail to save file.\nCan't save : "+file_path_name);
    }
}
