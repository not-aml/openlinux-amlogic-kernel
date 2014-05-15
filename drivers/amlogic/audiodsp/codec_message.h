#ifndef _CODEC_MESSAGE_HEADERS
#define _CODEC_MESSAGE_HEADERS


#define SUB_FMT_VALID 			(1<<1)
#define CHANNEL_VALID 			(1<<2)	
#define SAMPLE_RATE_VALID     	(1<<3)	
#define DATA_WIDTH_VALID     	(1<<4)	
struct frame_fmt
{

    int valid;
    int sub_fmt;
    int channel_num;
    int sample_rate;
    int data_width;
    int reversed[3];/*for cache aligned 32 bytes*/
};


struct frame_info
{

    int len;
    unsigned long  offset;/*steam start to here*/
    unsigned long  buffered_len;/*data buffer in  dsp,pcm datalen*/
    int reversed[1];/*for cache aligned 32 bytes*/
};

#endif

