//
// Created by Gabriele on 28/05/16.
//
#pragma once
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <assert.h>
#include <vector>
#include <string>


class es_objs_errors
{
public:

    using list_errors    = std::vector < std::string >;
    using list_errors_it = list_errors::const_iterator ;

    list_errors_it begin() const
    {
        return m_errors.begin();
    }

    list_errors_it end() const
    {
        return m_errors.end();
    }

    bool have_errors() const
    {
        return m_errors.size() != 0;
    }

    size_t count_errors() const
    {
        return m_errors.size();
    }

protected:

    void push_error(const std::string& error)
    {
        m_errors.push_back(error);
    }

    void clear_errors()
    {
        m_errors.clear();
    }

    list_errors m_errors;

};

class es_engine : public es_objs_errors
{
    public:

    //null ptr
    es_engine()
    {
        m_engine_obj = nullptr;
        m_engine_itf = nullptr;
    }

    //init device
    bool init()
    {
        //create engine
        SLuint32 result = slCreateEngine(&m_engine_obj, 0, nullptr, 0, nullptr, nullptr);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't create audio engine");
            return false;
        }

        // realize the engine
        result = (*m_engine_obj)->Realize(m_engine_obj, SL_BOOLEAN_FALSE);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't realize audio engine");
            return false;
        }

        // get the engine interface, which is needed in order to create other objects
        result = (*m_engine_obj)->GetInterface(m_engine_obj, SL_IID_ENGINE, &m_engine_itf);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't get audio engine");
            return false;
        }

        //ok
        return true;
    }

    void destoy()
    {
        //destoy
        (*m_engine_obj)->Destroy(m_engine_obj);
        //to null
        m_engine_obj = nullptr;
        m_engine_itf = nullptr;
    }

    //valid device
    bool is_valid() const
    {
        return (m_engine_obj != nullptr) && (m_engine_itf != nullptr);
    }

    bool can_destoy() const
    {
        return (m_engine_obj != nullptr);
    }

    SLObjectItf get_obj() const
    {
        return m_engine_obj;
    }

    SLEngineItf get_itf() const
    {
        return m_engine_itf;
    }

private:

    SLObjectItf m_engine_obj;
    SLEngineItf m_engine_itf;

};


class es_input_device : public es_objs_errors
{

public:

    struct es_input_meta_info
    {
        SLuint32 m_channels;
        SLuint32 m_samples_per_sec;
        SLuint32 m_bits_per_sample;
        SLuint32 m_count_queue;
        //bytes per second
        size_t get_bytes_per_second() const
        {
            return m_channels * m_samples_per_sec * (m_bits_per_sample/8);
        }
        size_t get_bits_per_second() const
        {
            return m_channels * m_samples_per_sec * m_bits_per_sample;
        }
    };

    struct es_buffer_read
    {
        size_t                 m_idx;
        size_t                 m_one_size;
        size_t                 m_n_buffer;
        std::vector < int8_t > m_buffer;

        void init (const es_input_meta_info& info,size_t seconds = 1)
        {
            m_idx = 0;
            m_one_size = info.m_samples_per_sec
                         * ( info.m_bits_per_sample / 8 )
                         * info.m_channels
                         * seconds;
            m_n_buffer = info.m_count_queue;
            m_buffer.resize( m_one_size * m_n_buffer, 0 );
        };

        void reset()
        {
            m_idx = 0;
        }

        size_t size() const
        {
            return m_n_buffer;
        }

        size_t index() const
        {
            return m_idx;
        }

        int8_t* current()
        {
            return &m_buffer[m_idx*m_one_size];
        }

        int8_t* get(size_t i)
        {
            return &m_buffer[i*m_one_size];
        }

        void next()
        {
            //next
            ++m_idx;
            //circle
            if(m_idx >= m_n_buffer) m_idx = 0;
        }

        void enqueue(SLAndroidSimpleBufferQueueItf bq)
        {
            (*bq)->Enqueue(bq,current(),m_one_size);
            next();
        }

        void enqueue_all(SLAndroidSimpleBufferQueueItf bq)
        {
            for(int i=0;i!=m_n_buffer;++i)
            {
                (*bq)->Enqueue(bq,get(i),m_one_size);
            }
        }
    };

    //null ptr
    es_input_device()
    {
        m_recorder_obj = nullptr;
        m_record_itf = nullptr;
        m_recorder_buffer_queue_itf = nullptr;
        //meta info
        m_info.m_channels = 0;
        m_info.m_samples_per_sec = 0;
        m_info.m_bits_per_sample = 0;
    }


    bool init(const es_engine& engine,
              const es_input_meta_info& info)
    {
        //type
        SLuint32 es_bits_type = 0 ;
        //select type
        switch (info.m_bits_per_sample)
        {
            case 8:  es_bits_type = SL_PCMSAMPLEFORMAT_FIXED_8;   break;
            case 16: es_bits_type = SL_PCMSAMPLEFORMAT_FIXED_16;  break;
            case 20: es_bits_type = SL_PCMSAMPLEFORMAT_FIXED_20;  break;
            case 32: es_bits_type = SL_PCMSAMPLEFORMAT_FIXED_32;  break;
            default:
                push_error("Bits per sample not supported");
                return false;
        }
        //samples per sec type
        SLuint32 es_samples_per_sec_type = 0 ;
        //samples per sec
        switch(info.m_samples_per_sec)
        {

            case 8000:   es_samples_per_sec_type = SL_SAMPLINGRATE_8;     break;
            case 11025:  es_samples_per_sec_type = SL_SAMPLINGRATE_11_025;break;
            case 16000:  es_samples_per_sec_type = SL_SAMPLINGRATE_16;    break;
            case 22050:  es_samples_per_sec_type = SL_SAMPLINGRATE_22_05; break;
            case 24000:  es_samples_per_sec_type = SL_SAMPLINGRATE_24;    break;
            case 32000:  es_samples_per_sec_type = SL_SAMPLINGRATE_32;    break;
            case 44100:  es_samples_per_sec_type = SL_SAMPLINGRATE_44_1;  break;
            case 48000:  es_samples_per_sec_type = SL_SAMPLINGRATE_48;    break;
            case 64000:  es_samples_per_sec_type = SL_SAMPLINGRATE_64;    break;
            case 88200:  es_samples_per_sec_type = SL_SAMPLINGRATE_88_2;  break;
            case 96000:  es_samples_per_sec_type = SL_SAMPLINGRATE_96;    break;
            case 192000: es_samples_per_sec_type = SL_SAMPLINGRATE_192;   break;
            default:
                push_error("Samples per second not supported");
                return false;
        }
        //save mata info
        m_info = info;
        //values
        SLuint32 result = 0;

        SLDataLocator_IODevice loc_dev =
                {
                        SL_DATALOCATOR_IODEVICE,
                        SL_IODEVICE_AUDIOINPUT,
                        SL_DEFAULTDEVICEID_AUDIOINPUT,
                        NULL
                };

        SLDataSource audio_src =
                {
                        &loc_dev,
                        NULL
                };

        SLDataLocator_AndroidSimpleBufferQueue loc_bq =
                {
                        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                        m_info.m_count_queue
                };

        SLDataFormat_PCM format_pcm =
                {
                        SL_DATAFORMAT_PCM,
                        m_info.m_channels,
                        es_samples_per_sec_type,
                        es_bits_type,
                        es_bits_type,
                        SL_SPEAKER_FRONT_CENTER,
                        SL_BYTEORDER_LITTLEENDIAN
                };

        SLDataSink audio_snk =
                {
                        &loc_bq,
                        &format_pcm
                };

        const SLInterfaceID id[2] =
                {
                        SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                        SL_IID_ANDROIDCONFIGURATION
                };

        const SLboolean req[2] =
                {
                        SL_BOOLEAN_TRUE,
                        SL_BOOLEAN_TRUE
                };

        //create audio recorder
        result =(*engine.get_itf())->CreateAudioRecorder(engine.get_itf(),
                                                         &m_recorder_obj,
                                                         &audio_src,
                                                         &audio_snk,
                                                         2,
                                                         id,
                                                         req);
        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't create audio recorder");
            return false;
        }

        // get the android configuration interface which is explicit
        result = (*m_recorder_obj)->GetInterface(m_recorder_obj,
                                                 SL_IID_ANDROIDCONFIGURATION,
                                                 (void*)&m_android_config);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't realize audio recorder");
            return false;
        }

        //use mic
        if(!force_use_recognition())
        {
            return false;
        }

        //realize recorder
        result = (*m_recorder_obj)->Realize(m_recorder_obj,  SL_BOOLEAN_FALSE);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't realize audio recorder");
            return false;
        }

        result = (*m_recorder_obj)->GetInterface(m_recorder_obj, SL_IID_RECORD, &m_record_itf);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't get audio recorder interface");
            return false;
        }

        result = (*m_recorder_obj)->GetInterface(m_recorder_obj,
                                                 SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                 &m_recorder_buffer_queue_itf);
        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't get audio recorder queues");
            return false;
        }

        return true;
    }

    void destroy()
    {
        //destoy
        (*m_recorder_obj)->Destroy(m_recorder_obj);
        //all to null
        m_recorder_obj              = nullptr;
        m_record_itf                = nullptr;
        m_recorder_buffer_queue_itf = nullptr;
    }

    //valid
    bool is_valid() const
    {
        return m_recorder_obj != nullptr  &&
               m_record_itf   != nullptr   &&
               m_recorder_buffer_queue_itf != nullptr;
    }

    //can destoy this?
    bool can_destoy() const
    {
        return m_recorder_obj != nullptr;
    }

    /* *
     * Call back example
     *
     *   void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
     *   {
     *
     *   }
     * */

    bool set_callback(void(* callback)(SLAndroidSimpleBufferQueueItf, void*),void* context)
    {

        int result = (*m_recorder_buffer_queue_itf)->RegisterCallback
                (
                m_recorder_buffer_queue_itf,
                callback,
                context
                );

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't set audio recorder callback");
            return false;
        }

        return true;
    }

    /**
     * start recording
     */
    bool start_recording()
    {
        int result = (*m_record_itf)->SetRecordState(m_record_itf,SL_RECORDSTATE_RECORDING);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't start the audio recording");
            return false;
        }

        return true;
    }

    /**
     * stop recording
     */
    bool stop_recording()
    {
        int result = (*m_record_itf)->SetRecordState(m_record_itf,SL_RECORDSTATE_STOPPED);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't stop the audio recording");
            return false;
        }

        return true;
    }

    /**
     * pause recording
     */
    bool pause_recording()
    {
        int result = (*m_record_itf)->SetRecordState(m_record_itf,SL_RECORDSTATE_STOPPED);

        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't pause the audio recording");
            return false;
        }

        return true;
    }

    //get meta info
    const es_input_meta_info& get_meta_info() const
    {
        return m_info;
    }

    //get queue
    SLAndroidSimpleBufferQueueItf get_raw_queue() const
    {
        return m_recorder_buffer_queue_itf;
    }

    //enqueue
    void enqueue_all(es_buffer_read& buffer)
    {
        buffer.enqueue_all(m_recorder_buffer_queue_itf);
    }

    void enqueue(es_buffer_read& buffer)
    {
        buffer.enqueue(m_recorder_buffer_queue_itf);
    }

private:

    SLObjectItf m_recorder_obj;
    SLRecordItf m_record_itf;
    SLAndroidConfigurationItf m_android_config;
    SLAndroidSimpleBufferQueueItf m_recorder_buffer_queue_itf;

    //meta info
    es_input_meta_info m_info;

    //force use mic
    bool force_use_recognition()
    {
        // use the configuration interface to configure the recorder before it's realized
        SLuint32 preset_value = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
        SLuint32 result = (*m_android_config)->SetConfiguration(m_android_config,
                                                                SL_ANDROID_KEY_RECORDING_PRESET,
                                                                &preset_value,
                                                                sizeof(SLuint32));
        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't force use the mic to audio recording");
            return false;
        }

        //test value to 0
        SLuint32 get_preset_value = SL_ANDROID_RECORDING_PRESET_NONE;
        //size
        SLuint32 preset_size = 2*sizeof(SLuint32); // intentionally too big
        //get info
        result = (*m_android_config)->GetConfiguration(m_android_config,
                                                       SL_ANDROID_KEY_RECORDING_PRESET,
                                                       &preset_size,
                                                       (void*)&get_preset_value);
        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't get android configuration (audio recording)");
            return false;
        }

        //test
        if (preset_value != get_preset_value)
        {
            push_error("Error retrieved recording preset  (audio recording)");
            return false;
        }

        return true;
    }
};
