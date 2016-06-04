#pragma once
#include <QObject>
#include <QIODevice>
#include <QByteArray>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QIODevice>

class q_audio_output : public QIODevice
{
    Q_OBJECT

public:

    explicit q_audio_output(QObject *parent = 0);
    bool init(int bits, int channels, int samplerate);
    bool set_device(QAudioDeviceInfo deviceInfo);

    void start();
    void set_volume(float volume);
    void output(const QByteArray & data);
    void output(const char* data,size_t size);
    void flush();
    void stop();

    //overload
    qint64 bytesAvailable() const Q_DECL_OVERRIDE;
    bool   isSequential() const Q_DECL_OVERRIDE;

private:
    //overload
    qint64 readData(char *data, qint64 maxlen) Q_DECL_OVERRIDE;
    qint64 writeData(const char *data, qint64 len) Q_DECL_OVERRIDE;
    //utility
    void reinit();

private:

    bool             m_initialized;
    QByteArray       m_buffer;
    QAudioFormat     m_format;
    QAudioDeviceInfo m_deviceInfo;
    QAudioOutput*    m_output;
    float            m_volume;

signals:

public slots:

    void notified();
    void stateChanged(QAudio::State state);

};
