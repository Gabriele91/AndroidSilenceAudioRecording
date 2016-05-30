//
// Created by Gabriele on 28/05/16.
//
#pragma once
#include <audio_engine.hpp>
//class dec
class sound_callback;
class sound_context;

//callback
class sound_callback
{
public:
    virtual void init(sound_context* context){};
    virtual void fail(){};
    virtual void update(SLAndroidSimpleBufferQueueItf bq) = 0;
};

//manager
class sound_context : public es_objs_errors
{
public:

    bool init(const es_input_device::es_input_meta_info& info)
    {
        //init engine
        if(!m_engine.init())
        {
            //push error
            push_error("Fail to init audio engine");
            //fail
            return false;
        }
        //init input device
        if(!m_input.init(m_engine,info))
        {
            //dealloc
            m_engine.destoy();
            //push error
            push_error("Fail to init input device");
            //fail to create input
            return false;
        }

        return true;
    }

    bool set_callback(sound_callback* callback)
    {
        //init
        callback->init(this);
        //reg
        if(!m_input.set_callback(callback_update,callback))
        {
            //fail
            callback->fail();
            //return false
            return false;
        }
        //success
        return true;
    }

    void destoy()
    {
        m_input.destroy();
        m_engine.destoy();
    }

    es_engine& get_engine()
    {
        return m_engine;
    }

    es_input_device& get_input()
    {
        return m_input;
    }

    bool global_have_errors() const
    {
        return  es_objs_errors::have_errors() ||
                m_engine.have_errors()        ||
                m_input.have_errors();
    }

    size_t global_count_errors() const
    {
        return count_errors() +
               m_engine.count_errors() +
               m_input.count_errors();
    }

protected:

    static void callback_update(SLAndroidSimpleBufferQueueItf bq, void *context)
    {
        ((sound_callback*)context)->update(bq);
    }

    es_engine m_engine;
    es_input_device m_input;
};