#ifndef Q_SETTINGS_H
#define Q_SETTINGS_H

#include <QWidget>
#include <QBasicTimer>
#include <QTimerEvent>

namespace Ui {
class q_settings;
}

class q_audio_player;
class q_android_silence_audio_recording;
class q_audio_server_listener;

class q_settings : public QWidget
{
    Q_OBJECT

public:

    explicit q_settings(q_android_silence_audio_recording *parent = 0);
    ~q_settings();
    void set_audio_server_listener(q_audio_server_listener* listener);

public slots:

    void back_to_device_list();
    void applay_settings();
    void save();
    void play_or_pause();
    void stop();
    void volume(int value);

private:

    //data buffer
    QVector< double >  m_x_values;
    QVector< double >  m_y_values;
    void append_sample(short sample);
    //update
    QBasicTimer*       m_timer { nullptr };
    void               timerEvent(QTimerEvent *etime);
    //attribute
    q_audio_player*                    m_player;
    q_audio_server_listener*           m_listener;
    q_android_silence_audio_recording* m_asar;
    Ui::q_settings*                    m_ui;
    QString                            m_last_path;
    //utility
    static QString build_item_string(const QString& android_id,const QString& imei, bool connected = true)
    {
        return "Android id: "+android_id+
                (imei.size() ?" | IMEI: "+imei:"")+
                " | status: "+
                (connected? "connected":"disconnected");
    }
    //clean ui
    void cleanup_info();

};

#endif // Q_SETTINGS_H