#include "q_audio_output.h"
#include <QDebug>
#include <QtEndian>
#include <math.h>
#define BUFFER_SIZE (64*1024)
//#define BE

q_audio_output::q_audio_output(QObject *parent) :
    QIODevice(parent),
    m_initialized(false),
    m_output(0),
    m_volume(0.0f)
{
}

bool q_audio_output::init(int bits, int channels, int samplerate)
{
    if (m_initialized)
    {
        return false;
    }
    if (bits != 16)
    {
        return false;
    }

    m_format.setSampleSize(bits);
    m_format.setChannelCount(channels);
    m_format.setSampleRate(samplerate);
    m_format.setCodec("audio/pcm");
#ifdef BE
    m_format.setByteOrder(QAudioFormat::BigEndian);
#else
    m_format.setByteOrder(QAudioFormat::LittleEndian);
#endif
    //m_format.setSampleType(QAudioFormat::UnSignedInt);
    m_format.setSampleType(QAudioFormat::SignedInt);

    m_initialized = set_device(QAudioDeviceInfo::defaultOutputDevice());
    return m_initialized;
}

bool q_audio_output::set_device(QAudioDeviceInfo deviceInfo)
{
    if (!deviceInfo.isFormatSupported(m_format))
    {
        qDebug() << "Format not supported!";
        return false;
    }
    m_deviceInfo = deviceInfo;
    //init
    reinit();
    //true
    return true;
}

void q_audio_output::reinit()
{
    bool running = false;
    if (m_output && m_output->state() != QAudio::StoppedState)
    {
        running = true;
    }
    //this
    stop();
    // Reinitialize audio output
    delete m_output;
    m_output = 0;
    m_output = new QAudioOutput(m_deviceInfo, m_format, this);
    // size buffer
    qDebug() << "QAudioOutput init";
    // m_output->setBufferSize(BUFFER_SIZE);
    // Set constant values to new audio output
    QObject::connect(m_output,
            &QAudioOutput::notify,
            this,
            &q_audio_output::notified);
    QObject::connect(m_output,
            &QAudioOutput::stateChanged,
            this,
            &q_audio_output::stateChanged);
    //running
    if (running) start();
}

void q_audio_output::start()
{
    if (m_output == 0 || m_output->state() != QAudio::StoppedState)
    {
        return;
    }
    //this
    open(QIODevice::ReadOnly);
    //start
    m_buffer.clear();
    m_output->start(this);
    m_output->suspend();
}

void q_audio_output::set_volume(float volume)
{
    m_volume = volume;
}

void q_audio_output::output(const QByteArray & data)
{
    if (m_output && m_output->state() != QAudio::StoppedState)
    {
        // Append input data to the end of buffer
        m_buffer.append(data);

        // Check if our buffer has grown too large
        if (m_buffer.length() > 2*BUFFER_SIZE)
        {
            qDebug() << "Flush...";
            // There could be a better way to handle this
            flush();
        }

        // If audio is suspended and buffer is full, resume
        if (m_output->state() == QAudio::SuspendedState)
        {
            if (m_buffer.length() >= BUFFER_SIZE)
            {
                qDebug() << "Resuming...";
                m_output->resume();
            }
        }
    }
}
void q_audio_output::output(const char* data,size_t size)
{
    if (m_output && m_output->state() != QAudio::StoppedState)
    {
        // Append input data to the end of buffer
        m_buffer.append(data,size);

        // Check if our buffer has grown too large
        if (m_buffer.length() > 2*BUFFER_SIZE)
        {
            qDebug() << "Flush...";
            // There could be a better way to handle this
            flush();
        }

        // If audio is suspended and buffer is full, resume
        if (m_output->state() == QAudio::SuspendedState)
        {
            if (m_buffer.length() >= BUFFER_SIZE)
            {
                qDebug() << "Resuming...";
                m_output->resume();
            }
        }
    }
}


void q_audio_output::flush()
{
    // Flushing buffers is a bit tricky...
    // Don't modify this unless you're sure
    this->stop();
    m_output->reset();
    this->start();
}

void q_audio_output::stop()
{
    if (m_output && m_output->state() != QAudio::StoppedState) {
        // Stop audio output
        m_output->stop();
        m_buffer.clear();
        this->close();
    }
}

static void apply_s16le_volume(float volume, uchar *data, int datalen)
{
    int samples = datalen/2;
    float mult = qBound(0.0f, powf(10.0,0.05*volume), 1.0f);

    for (int i=0; i<samples; i++)
    {
        qint16 val = qFromLittleEndian<qint16>(data+i*2)*mult;
        qToLittleEndian<qint16>(val, data+i*2);
    }
}

static void apply_s16be_volume(float volume, uchar *data, int datalen)
{
    int samples = datalen/2;
    float mult = qBound(0.0f, powf(10.0,0.05*volume), 1.0f);

    for (int i=0; i<samples; i++)
    {
        qint16 val = qFromBigEndian<qint16>(data+i*2)*mult;
        qToBigEndian<qint16>(val, data+i*2);
    }
}

qint64 q_audio_output::readData(char *data, qint64 maxlen)
{
    qDebug() << "device read...";
    // Calculate output length, always full samples
    int outlen = qMin(m_buffer.length(), (int)maxlen);

    if (outlen%2 != 0)
    {
        outlen += 1;
    }

    memcpy(data, m_buffer.data(), outlen);
#ifdef BE
    apply_s16be_volume(m_volume, (uchar *)data, outlen);
#else
    apply_s16le_volume(m_volume, (uchar *)data, outlen);
#endif
    m_buffer.remove(0, outlen);
    return outlen;
}

qint64 q_audio_output::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 q_audio_output::bytesAvailable() const
{
    return m_buffer.length() + QIODevice::bytesAvailable();
}

bool q_audio_output::isSequential() const
{
    return true;
}

void q_audio_output::notified()
{
    qDebug() << "notified";
}

void q_audio_output::stateChanged(QAudio::State state)
{
    qDebug() << "state = " << state;
    // Start buffering again in case of underrun...
    // Required on Windows, otherwise it stalls idle
    if (state == QAudio::IdleState &&
        m_output->error() == QAudio::UnderrunError)
    {
        // This check is required, because Mac OS X underruns often
        if (m_buffer.length() < BUFFER_SIZE)
        {
            m_output->suspend();
        }
    }
}
