//
// Created by Gabriele on 29/05/16.
//
#pragma  once
#include <cstdio>
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#define BYTE_ORDER BIG_ENDIAN

struct input_meta_info
{
    unsigned int m_channels;
    unsigned int m_samples_per_sec;
    unsigned int m_bits_per_sample;
    //bytes per second

    size_t get_bytes_per_second() const
    {
        return m_channels * m_samples_per_sec * (m_bits_per_sample/8);
    }

    size_t get_bytes_per_sample() const
    {
        return m_channels * (m_bits_per_sample/8);
    }

    size_t get_bits_per_second() const
    {
        return m_channels * m_samples_per_sec * m_bits_per_sample;
    }

    size_t get_bits_per_sample() const
    {
        return m_channels * m_bits_per_sample;
    }
};

#ifdef _MSC_VER
    #define ASPACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
    #define ASPACKED( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

class wav_riff
{
public:
    ASPACKED(struct riff_header
            {
                char         m_chunk_id[4];
                unsigned int m_chunk_size;
                char         m_format[4];
            });

    ASPACKED(struct fmt_header
             {
                 char           m_subchunk1_id[4];
                 unsigned int   m_subchunk1_size;
                 unsigned short m_audio_format;
                 unsigned short m_num_channels;
                 unsigned int   m_sample_rate;
                 unsigned int   m_byte_rate;  // sample rate * channels * bits sample /8
                 unsigned short m_block_align;// channels * bits sample /8
                 unsigned short m_bits_per_sample;
             });

    ASPACKED(struct date_header_write
             {
                 char         m_subchunk2_id[4];
                 unsigned int m_subchunk2_size;
             });

    ASPACKED(struct date_header_read
             {
                 char         m_subchunk2_id[4];
                 unsigned int m_subchunk2_size;
                 char         m_data[1];
             });

    enum endianness
    {
        LE_MODE,
        BE_MODE
    };

    void init(FILE* file,const input_meta_info& info, endianness mode)
    {
        //save ptr
        m_file             = file;
        m_bits_per_sample  = info.m_bits_per_sample;
        //init
        riff_header riff =
                {
                        {'R','I', 'F', 'F'},
                        sizeof(riff_header),
                        {'W','A', 'V', 'E'}
                };

        ////////////////////////////////
        //BE?
        if(mode == BE_MODE)
        {
            //RIFF to RIFX
            riff.m_chunk_id[3] = 'X';
        }
        ////////////////////////////////

        fmt_header fmt =
                {
                        {'f','m','t', ' '},
                        (unsigned int)  16,                          //size of the rest of subchunk
                        (unsigned short)1,                           //format pcm = 1
                        (unsigned short)info.m_channels,             //channels
                        (unsigned int)  info.m_samples_per_sec,      //sample rate
                        (unsigned int)  info.get_bytes_per_second(), //sample rate * channels * bits sample /8
                        (unsigned short)info.get_bytes_per_sample(), //channels * bits sample / 8
                        (unsigned short)info.m_bits_per_sample,      //bits sample
                };

        date_header_write data =
        {
                {'d','a','t', 'a'},
                0
        };
        //write header riff
        std::fwrite(&riff, sizeof(riff_header), 1, m_file);
        //write header fmt
        std::fwrite(&fmt,  sizeof(fmt_header), 1, m_file);
        //write header data
        std::fwrite(&data, sizeof(date_header_write), 1, m_file);
    }


    void append(void* buffer, size_t size,endianness mode)
    {
#if BYTE_ORDER == LITTLE_ENDIAN
        if(mode == BE_MODE)
#elif BYTE_ORDER == BIG_ENDIAN
        if(mode == LE_MODE)
#endif
        {
            //LE to BE
            switch(m_bits_per_sample)
            {
                case 16: ending_conv_sound_buff16((unsigned char*)buffer,size); break;
                case 32: ending_conv_sound_buff32((unsigned char*)buffer,size); break;
                case 8:   /* none */  break;
                case 24:  /* todo */  break;
                default:  /* error */ break;
            }
        }
        //..
        std::fwrite(buffer,size,1,m_file);
    }

    void complete()
    {
        //at end get file size
        size_t size_file = std::ftell(m_file);
        ////////////////////////////////////////////////////////////////////////////////////
        std::fseek(m_file,offsetof(riff_header,m_chunk_size),SEEK_SET);
        //write size of the rest file
        unsigned int riff_chunk_size = size_file - 8;
        std::fwrite(&riff_chunk_size,sizeof(riff_chunk_size), 1,m_file);
        ////////////////////////////////////////////////////////////////////////////////////
        //set pos
        const size_t size_data_pos =    sizeof(riff_header)
                                      + sizeof(fmt_header)
                                      + offsetof(date_header_write,m_subchunk2_size);
        std::fseek(m_file,size_data_pos,SEEK_SET);
        //compute size
        unsigned int size_data =   size_file
                                 - sizeof(riff_header)
                                 - sizeof(fmt_header)
                                 - sizeof(date_header_write);
        //write size
        std::fwrite(&size_data,sizeof(size_data), 1,m_file);
        ////////////////////////////////////////////////////////////////////////////////////
        //set to end
        std::fseek(m_file,0,SEEK_END);
    }

private:

    static unsigned short endian_uint16_conversion(unsigned short word)
    {
        return ((word >> 8) & 0x00FF) | ((word << 8) & 0xFF00);
    }

    static unsigned int endian_uint32_conversion(unsigned int dword)
    {
        return ((dword >> 24) & 0x000000FF) | ((dword >> 8) & 0x0000FF00) | ((dword << 8) & 0x00FF0000) | ((dword << 24) & 0xFF000000);
    }

    static void ending_conv_sound_buff16(unsigned char* u8_buffer,long u8_size)
    {
        while (u8_size > 0)
        {
            ///////////////
            unsigned short* u16_buffer = (unsigned short*)u8_buffer;
            (*u16_buffer) = endian_uint16_conversion(*u16_buffer);
            ///////////////
            u8_buffer += 2;
            u8_size   -= 2;
        }
    }

    static void ending_conv_sound_buff32(unsigned char* u8_buffer,long u8_size)
    {
        while (u8_size > 0)
        {
            ///////////////
            unsigned int* u32_buffer = (unsigned int*)u8_buffer;
            (*u32_buffer) = endian_uint32_conversion(*u32_buffer);
            ///////////////
            u8_buffer += 4;
            u8_size   -= 4;
        }
    }

    //current file
    size_t m_bits_per_sample  { 0 };
    FILE*  m_file             { nullptr };
};