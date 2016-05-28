//
// Created by Gabriele on 28/05/16.
//
#pragma once
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Platform.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidMetadata.h>
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
        int result = slCreateEngine(&m_engine_obj, 0, NULL, 0, NULL, NULL);

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
        //save mata info
        m_info = info;
        //values
        int result = 0;

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
                        info.m_count_queue
                };

        SLDataFormat_PCM format_pcm =
                {
                        SL_DATAFORMAT_PCM,
                        info.m_channels,
                        info.m_samples_per_sec,
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

        const SLInterfaceID id[1] =
                {
                        SL_IID_ANDROIDSIMPLEBUFFERQUEUE
                };

        const SLboolean req[1] =
                {
                        SL_BOOLEAN_TRUE
                };

        //create audio recorder
        result =(*engine.get_itf())->CreateAudioRecorder(engine.get_itf(),
                                                         &m_recorder_obj,
                                                         &audio_src,
                                                         &audio_snk,
                                                         1,
                                                         id,
                                                         req);
        //errors?
        if(result != SL_RESULT_SUCCESS)
        {
            push_error("Can't create audio recorder");
            return false;
        }

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
               m_record_itf != nullptr   &&
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

    bool set_callback(void(* callback)(SLAndroidSimpleBufferQueueItf bq, void *context),void* context)
    {

        int result = (*m_recorder_buffer_queue_itf)->RegisterCallback
                (
                m_recorder_buffer_queue_itf,
                callback,
                context
                );

        //errors?
        if(SL_RESULT_PARAMETER_INVALID == result)
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
        if(SL_RESULT_PARAMETER_INVALID == result)
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
        if(SL_RESULT_PARAMETER_INVALID == result)
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
        if(SL_RESULT_PARAMETER_INVALID == result)
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

private:

    SLObjectItf m_recorder_obj;
    SLRecordItf m_record_itf;
    SLAndroidSimpleBufferQueueItf m_recorder_buffer_queue_itf;

    //meta info
    es_input_meta_info m_info;
};
