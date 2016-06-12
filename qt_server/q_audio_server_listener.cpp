#include <q_audio_server_listener.h>
#include <QMessageBox>
#include <QFile>
#include <QCryptographicHash>
//not use opus
//#define USE_RAW_AUDIO

q_audio_server_listener::q_audio_server_listener()
{
}

q_audio_server_listener::~q_audio_server_listener()
{
    if(m_decoder) opus_decoder_destroy(m_decoder);
}


void q_audio_server_listener::init(const input_meta_info& info)
{
    //default metainfo
    m_info = info;
    //init already called
    if(m_decoder) opus_decoder_destroy(m_decoder);
    //alloc decoder
    m_decoder = opus_decoder_create(m_info.m_samples_per_sec, m_info.m_channels, &m_error);
    //set the BITRATE
    #define BITRATE 12000
    opus_decoder_ctl(m_decoder, OPUS_SET_BITRATE(BITRATE));
    //alloc buffer
    m_buf_dec.resize(m_info.m_channels*m_info.m_samples_per_sec*m_info.m_bits_per_sample/8);
}

void q_audio_server_listener::incoming_connection(rak_server& server, const RakNet::AddressOrGUID addr)
{
    //save addr
    m_addr = addr;
    //connected
    m_state = S_CONN;
    //is connected
    m_connected = true;
    //callback
    call_connection_cb();
}

void q_audio_server_listener::end_connection(rak_server&, const RakNet::AddressOrGUID)
{
    //connected
    m_state = S_DISC;
    //not connected
    m_connected = false;
    //close file
    if(output_file_is_open()){ close_output_file_ui(); }
    //callback
    call_connection_cb();
}

void q_audio_server_listener::fail_connection(rak_server&, const RakNet::AddressOrGUID)
{
    //connected
    m_state = S_DISC;
    //not connected
    m_connected = false;
    //close file
    if(output_file_is_open()){ close_output_file_ui(); }
    //callback
    call_connection_cb();
}


void q_audio_server_listener::get_raw_voice(rak_server& server,
                                            const RakNet::AddressOrGUID addrs,
                                            RakNet::BitStream& stream)
{

#ifdef USE_RAW_AUDIO
    //buffer size
    unsigned int size = stream.GetNumberOfUnreadBits() / 8;
    //alloc buffer
    std::vector < unsigned char > buffer(size,0);
    //read buffer
    stream.ReadBits(buffer.data(), size*8);
    //debug
    qDebug() << "sound arrived: " << size;
    //append
    applay_to_output_buffer((const char *)buffer.data(), buffer.size());
    //append to file
    append_to_file((const char *)buffer.data(), size, wav_riff::BE_MODE);
#else
    //ptr buffer
    int         buff16_size = (int)m_buf_dec.size() / sizeof(opus_int16);
    opus_int16* buff16_ptr  = (opus_int16*)m_buf_dec.data();
    //get count of blocks
    unsigned int buff16_div;
    stream.Read(buff16_div);
    //for each
    for(int i=0;i!=buff16_div;++i)
    {
        //get size of a block;
        int block_size = 0;
        stream.Read(block_size);
        //alloc buffer
        std::vector < unsigned char > buffer(block_size,0);
        stream.ReadBits(buffer.data(), block_size*8);
        //get
        int samples_out =
        opus_decode(m_decoder,
                    buffer.data(),
                    block_size,
                    buff16_ptr,
                    buff16_size,
                    0);
        //test
        assert(samples_out >= 0);
        //dec
        buff16_size -= samples_out;
        //next
        buff16_ptr  += samples_out;
    }
    //data size
    size_t data_size = m_buf_dec.size()-(buff16_size * sizeof(opus_int16));
    //audio in debug
    qDebug() << "sound arrived: " << data_size;
    //sound to output buffer
    applay_to_output_buffer((const char*)m_buf_dec.data(),data_size);
    //write file buffer
    append_to_file((const char*)m_buf_dec.data(),data_size);
#endif
}


void q_audio_server_listener::update(rak_server& server)
{

}

void q_audio_server_listener::get_imei_and_android_id(rak_server& server,
                                                      const RakNet::AddressOrGUID,
                                                      const char* imei,
                                                      const char* android_id)
{
    m_imei = imei;
    m_android_id = android_id;
}

atomic_listener_state& q_audio_server_listener::state()
{
    return m_state;
}

RakNet::AddressOrGUID& q_audio_server_listener::address()
{
    return m_addr;
}

void q_audio_server_listener::send_start(rak_server& server)
{
    server.mutex().lock();
    //send type
    server.send_start_msg(m_addr);
    //start
    m_state = S_REC;
    //end
    server.mutex().unlock();
}

void q_audio_server_listener::send_pause(rak_server& server)
{
    server.mutex().lock();
    //send type
    server.send_pause_msg(m_addr);
    //pause
    m_state = S_PAUSE;
    //end
    server.mutex().unlock();
}

void q_audio_server_listener::send_stop(rak_server& server)
{
    server.mutex().lock();
    //send type
    server.send_stop_msg(m_addr);
    //stop
    m_state = S_STOP;
    //end
    server.mutex().unlock();
}

void q_audio_server_listener::send_meta_info(rak_server& server)
{
    server.mutex().lock();
    //send tipe
    server.send_config_msg(m_addr, m_info.m_channels, m_info.m_samples_per_sec, m_info.m_bits_per_sample);
    //send info
    m_state = S_INFO;
    //end
    server.mutex().unlock();
}

void q_audio_server_listener::reset_state()
{
    //no change state
    if(m_state == S_DISC) return;
    //else return to connect
    m_state = S_CONN;

}

const std::string& q_audio_server_listener::get_imei() const
{
    return m_imei;
}

const std::string& q_audio_server_listener::get_android_id() const
{
    return m_android_id;
}

input_meta_info q_audio_server_listener::get_meta_info()
{
    return m_info;
}

bool q_audio_server_listener::connected() const
{
    return m_connected;
}

void q_audio_server_listener::set_callback_of_connection_changed_the_state(std::function<void(bool)> callback)
{
    m_connection_cb = callback;
}

bool q_audio_server_listener::open_output_file(const std::string& path,
                                               const wav_riff::info_fields& riff_meta_info)
{
    //open file
    m_file = fopen(path.c_str(),"wb");
    //open
    if(m_file)
    {
        //save path
        m_path = QString(path.c_str());
        //set file
        m_wav.set_file(m_file);
        //init file
        m_wav.init(m_info, wav_riff::LE_MODE, riff_meta_info);
        //ok
        return true;
    }

    return false;
}

const QString& q_audio_server_listener::get_output_file_path() const
{
    return m_path;
}

q_audio_server_listener::file_save_state q_audio_server_listener::close_output_file(bool create_md5)
{
    if(m_file)
    {
        //compute size
        m_wav.complete();
        //close
        fclose(m_file);
        //to null
        m_file = nullptr;
        //
        if(create_md5 && !create_file_md5(get_output_file_path()))
        {
            return S_F_SAVE_FAIL_SAVE_MD5;
        }
        //ok
        return S_F_SAVE_OK;
    }

    return S_F_SAVE_FAIL;
}



//save file
q_audio_server_listener::file_save_state q_audio_server_listener::close_output_file_ui(QWidget* parent,bool create_md5)
{
    //path
    auto& file_path_name = get_output_file_path();
    //close
    auto save_state = close_output_file(create_md5);
    //cases
    switch(save_state)
    {
        case q_audio_server_listener::S_F_SAVE_FAIL:
            QMessageBox::about(parent,"Abort","Fail to save file.\nCan't save : "+file_path_name);
        break;
        case q_audio_server_listener::S_F_SAVE_FAIL_SAVE_MD5:
             QMessageBox::about(parent,"Abort","Fail to create hash file.\nCan't save : "+file_path_name+".md5");
        break;
        default: break;
    }
    return save_state;
}

bool q_audio_server_listener::output_file_is_open() const
{
    return m_file != nullptr;
}


void q_audio_server_listener::set_output_buffer(QByteArray* buffer)
{
    m_buffer = buffer;
}

void q_audio_server_listener::disable_output_buffer()
{
    set_output_buffer(nullptr);
}

//append
void q_audio_server_listener::applay_to_output_buffer(const char* data,size_t size)
{
    if(m_buffer) m_buffer->append(data,size);
}


//utility
void q_audio_server_listener::call_connection_cb() const
{
    if(m_connection_cb)  m_connection_cb(m_connected);
}

//append
void q_audio_server_listener::append_to_file(const char* buffer,
                                             size_t size,
                                             wav_riff::endianness mode)
{
     if(m_file) m_wav.append_stream(buffer,size,mode);
}

//utils
bool q_audio_server_listener::create_file_md5(const QString &file_path)
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
