#ifndef Q_SETTINGS_H
#define Q_SETTINGS_H

#include <QWidget>

namespace Ui {
class q_settings;
}

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
    void select_path();
    void save();
    void play_or_pause();
    void stop();

private:

    q_audio_server_listener*           m_listener;
    q_android_silence_audio_recording* m_asar;
    Ui::q_settings*                    m_ui;

    QString build_item_string(const QString& android_id,const QString& imei, bool connected = true)
    {
        return "Android id: "+android_id+
                (imei.size() ?" | IMEI: "+imei:"")+
                " | status: "+
                (connected? "connected":"disconnected");
    }
    void cleanup_info();

};

#endif // Q_SETTINGS_H
