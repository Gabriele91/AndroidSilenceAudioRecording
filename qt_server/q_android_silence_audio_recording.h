#pragma once
#include <QMainWindow>
#include <rak_server.hpp>
#include <q_audio_server_listener.h>
#include <q_list_server_listener.h>
#include <q_settings.h>
namespace Ui {
class q_android_silence_audio_recording;
}


class q_android_silence_audio_recording : public QMainWindow
{
    Q_OBJECT

public:

    explicit q_android_silence_audio_recording(QWidget *parent = 0);
    ~q_android_silence_audio_recording();

    //change ui
    void show_settings(q_audio_server_listener* listener);
    void show_list_device();
    rak_server& get_rak_server();

public slots:

    void itemClicked(QListWidgetItem* item);

protected:

    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;

private:

    rak_server                             m_rak_server;
    q_settings*                            m_settings;
    q_list_server_listener                 m_list_listener;
    QPoint                                 m_drag_position;
    Ui::q_android_silence_audio_recording* m_ui;


};
