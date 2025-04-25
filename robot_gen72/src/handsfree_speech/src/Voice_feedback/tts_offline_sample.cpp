/*
* 语音合成（Text To Speech，TTS）技术能够自动将任意文字实时转换为连续的
* 自然语音，是一种能够在任何时间、任何地点，向任何人提供语音信息服务的
* 高效便捷手段，非常符合信息时代海量数据、动态更新和个性化查询的需求。
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../../include/qtts.h"
#include "../../include/msp_cmn.h"
#include "../../include/msp_errors.h"
#include <iostream>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <time.h>
#include "../../include/play_audio.h"
#include "alsa/asoundlib.h"
#include <cstdlib>
#include <ctime>
#include<ros/package.h>

#define random(a,b) (rand()%(b-a)+a)

typedef int SR_DWORD;
typedef short int SR_WORD ;

ros::Publisher pub;

static const std::string s_filename = ros::package::getPath("handsfree_speech")+"/res/tts_sample.wav";
static const std::string s_filename_move = ros::package::getPath("handsfree_speech")+"/res/tts_sample_move.wav";
static const std::string s_session_begin_params = "engine_type = local,voice_name=xiaoyan, text_encoding = UTF8, tts_res_path = fo|"+ros::package::getPath("handsfree_speech")+"/cfg/msc/res/tts/xiaoyan.jet;fo|"+ros::package::getPath("handsfree_speech")+"/cfg/msc/res/tts/common.jet, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";
//const char* workspace =fisheye_setting.c_str();

int ifwake=0;

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr
{
	char            riff[4];                // = "RIFF"
	int				size_8;                 // = FileSize - 8
	char            wave[4];                // = "WAVE"
	char            fmt[4];                 // = "fmt "
	int				fmt_size;				// = 下一个结构体的大小 : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = 通道数 : 1
	int				samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	int				avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	short int       bits_per_sample;        // = 量化比特数: 8 | 16

	char            data[4];                // = "data";
	int				data_size;              // = 纯数据长度 : FileSize - 44 
} wave_pcm_hdr;

/* 默认wav音频头部数据 */
wave_pcm_hdr default_wav_hdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};
const char* session_begin_params = s_session_begin_params.c_str();

char const * filename = s_filename.c_str(); //合成的语音文件名称
char const * filename_move = s_filename_move.c_str(); //合成的语音文件名称
time_t rawtime;
struct tm *info;
char buffer[80];

int set_pcm_play(FILE *fp, WavHeader *wav_header)
{
    int rc;
    int ret;
    int size;
    snd_pcm_t* handle; //PCI设备句柄
    snd_pcm_hw_params_t* params;//硬件信息和PCM流配置
    unsigned int val;
    int dir=0;
    snd_pcm_uframes_t frames;
    char *buffer;
    int channels=wav_header->wChannels;
    int frequency=wav_header->nSamplesPersec;
    int bit=wav_header->wBitsPerSample;
    int datablock=wav_header->wBlockAlign;
    //unsigned char ch[100]; //用来存储wav文件的头信息



    rc=snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if(rc<0)
    {
        perror("\nopen PCM device failed:");
        exit(1);
    }


    snd_pcm_hw_params_alloca(&params); //分配params结构体
    if(rc<0)
    {
        perror("\nsnd_pcm_hw_params_alloca:");
        exit(1);
    }
     rc=snd_pcm_hw_params_any(handle, params);//初始化params
    if(rc<0)
    {
        perror("\nsnd_pcm_hw_params_any:");
        exit(1);
    }
    rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //初始化访问权限
    if(rc<0)
    {
        perror("\nsed_pcm_hw_set_access:");
        exit(1);
    }

    //采样位数
    switch(bit/8)
    {
        case 1:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
                break ;
        case 2:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
                break ;
        case 3:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S24_LE);
                break ;

    }
    rc=snd_pcm_hw_params_set_channels(handle, params, channels); //设置声道,1表示单声>道，2表示立体声
    if(rc<0)
    {
        perror("\nsnd_pcm_hw_params_set_channels:");
        exit(1);
    }
    val = frequency;
    rc=snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir); //设置>频率
    if(rc<0)
    {
        perror("\nsnd_pcm_hw_params_set_rate_near:");
        exit(1);
    }

    rc = snd_pcm_hw_params(handle, params);
    if(rc<0)
    {
        perror("\nsnd_pcm_hw_params: ");
        exit(1);
    }

    rc=snd_pcm_hw_params_get_period_size(params, &frames, &dir); /*获取周期长度*/
    if(rc<0)
    {
        perror("\nsnd_pcm_hw_params_get_period_size:");
        exit(1);
    }

    size = frames * datablock; /*4 代表数据快长度*/

    buffer =(char*)malloc(size);
    fseek(fp,58,SEEK_SET); //定位歌曲到数据区

    while (1)
    {
        memset(buffer,0,sizeof(buffer));
        ret = fread(buffer, 1, size, fp);
        if(ret == 0)
        {
            //printf("歌曲写入结束\n");
            break;
        }
         else if (ret != size)
        {
          //printf("歌曲写入结束\n");
          break;
        }

        // 写音频数据到PCM设备
        snd_pcm_writei(handle, buffer, frames);
        if (ret == -EPIPE){
          /* EPIPE means underrun */
          fprintf(stderr, "underrun occurred\n");
          //完成硬件参数设置，使设备准备好
          snd_pcm_prepare(handle);
        }else if (ret < 0){
           fprintf(stderr,
           "error from writei: %s\n",
           snd_strerror(ret));
        }/*else if(ret > 0 || r == -EAGAIN){
          snd_pcm_wait
        }*/
        usleep(2 * 1000);
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    return 0;
}



int play_wav(char *path){
    FILE *fp;
    WavHeader wav_header;
    fp = fopen(path, "rb");
    if(fp == NULL){
      return -1;
    }
    int nread = fread(&wav_header, 1, sizeof(wav_header), fp);
    set_pcm_play(fp, &wav_header);
}

/* 文本合成 */
int text_to_speech(const char* src_text, const char* des_path, const char* params)
{
	int          ret          = -1;
	FILE*        fp           = NULL;
	const char*  sessionID    = NULL;
	unsigned int audio_len    = 0;
	wave_pcm_hdr wav_hdr      = default_wav_hdr;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	if (NULL == src_text || NULL == des_path)
	{
		printf("params is error!\n");
		return ret;
	}
	fp = fopen(des_path, "wb");
	if (NULL == fp)
	{
		printf("open %s error.\n", des_path);
		return ret;
	}
	/* 开始合成 */
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionBegin failed, error code: %d.\n", ret);
		fclose(fp);
		return ret;
	}
	ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSTextPut failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		fclose(fp);
		return ret;
	}
	printf("正在合成 ...\n");
	fwrite(&wav_hdr, sizeof(wav_hdr) ,1, fp); //添加wav音频头，使用采样率为16000
	while (1) 
	{
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{
			fwrite(data, audio_len, 1, fp);
		    wav_hdr.data_size += audio_len; //计算data_size大小
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
	}
	printf("\n");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSAudioGet failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		fclose(fp);
		return ret;
	}
	/* 修正wav文件头数据的大小 */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);
	
	/* 将修正过的数据写回文件头部,音频文件为wav格式 */
	fseek(fp, 4, 0);
	fwrite(&wav_hdr.size_8,sizeof(wav_hdr.size_8), 1, fp); //写入size_8的值
	fseek(fp, 40, 0); //将文件指针偏移到存储data_size值的位置
	fwrite(&wav_hdr.data_size,sizeof(wav_hdr.data_size), 1, fp); //写入data_size的值
	fclose(fp);
	fp = NULL;
	/* 合成完毕 */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionEnd failed, error code: %d.\n",ret);
	}

	return ret;
}

void chatterCallback(const std_msgs::String::ConstPtr& msg)
{
  ROS_INFO("I heard: [%s]", msg->data.c_str());
  int ret;
  time( &rawtime );
  info = localtime( &rawtime );
  int todo_id = atoi(msg->data.c_str());
  if(todo_id==99999)
  {
	char* text;
      int x =random(0, 5);
      if(x==1)
	{
		 text =(char*)"有什么事情"; //合成文本
	}
      else if(x==2)
	{
		 text =(char*)"怎么啦"; //合成文本
	}
      else if(x==3)
	{
		 text =(char*)" 来了"; //合成文本
	}
      else
	{
		 text =(char*)" 我在"; //合成文本
	}
      printf("收到唤醒指令\n");
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      ifwake=1;
      std_msgs::String msg_pub;
      msg_pub.data ="stop";
      pub.publish(msg_pub);
      play_wav((char*)filename_move);
      msg_pub.data ="tiago";
      pub.publish(msg_pub);
  }
  if(todo_id==10006&&ifwake==1)
  {
      strftime(buffer,80,"现在时间是:%Y年%m月%e日，%H点%M分%S秒", info);//以年月日_时分秒的形式表示当前时间
      const char* text =buffer; //合成文本
      ret =text_to_speech(text, filename, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("收到获取时间指令\n");
      printf("合成完毕\n");
      play_wav((char*)filename);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="stop";
      pub.publish(msg_pub);
  }
  if(todo_id==10001&&ifwake==1)
  {
      const char* text ="收到指令，请注意避让"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到前进指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="forward";
      pub.publish(msg_pub);
  }
  if(todo_id==55555)
  {
      //当收到休眠命令时，进行休息；
      const char* text ="好的，有事再唤醒我"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
        printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("好的，有事再唤醒我\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="rest";
      pub.publish(msg_pub);
      
  }
  if(todo_id==10006&&ifwake==1)
  {
      strftime(buffer,80,"现在时间是:%Y年%m月%e日，%H点%M分%S秒", info);//以年月日_时分秒的形式表示当前时间
      const char* text =buffer; //合成文本
      ret =text_to_speech(text, filename, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
        printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("收到获取时间指令\n");
      printf("合成完毕\n");
      play_wav((char*)filename);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="stop";
      pub.publish(msg_pub);
  }
  if(todo_id==10001&&ifwake==1)
  {
      const char* text ="收到指令，请注意避让"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
        printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到前进指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="forward";
      pub.publish(msg_pub);
  }
  if(todo_id==10002&&ifwake==1)
  {
      const char* text ="收到指令，请注意，倒车"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到倒车指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="backward";
      pub.publish(msg_pub);
  }
  if(todo_id==10003&&ifwake==1)
  {
      const char* text ="收到指令，正在转弯"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到左转指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="left";
      pub.publish(msg_pub);
  }
  if(todo_id==10004&&ifwake==1)
  {
      const char* text ="收到指令，正在转弯"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到右转指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="right";
      pub.publish(msg_pub);
  }
  if(todo_id==10005&&ifwake==1)
  {
      const char* text ="收到指令，进入休眠"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到休眠指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="shutdown";
      pub.publish(msg_pub);
  }
  if(todo_id==10007&&ifwake==1)
  {
      const char* text ="收到指令，正在前往主展台"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到前往A点指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="A";
      pub.publish(msg_pub);
  }
  if(todo_id==10008&&ifwake==1)
  {
      const char* text ="收到指令，正在前往区块链展台"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到前往B点指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="B";
      pub.publish(msg_pub);
  }
  if(todo_id==10009&&ifwake==1)
  {
      const char* text ="收到指令，正在前往三维建模展台"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到前往C点指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="C";
      pub.publish(msg_pub);
  }
  if(todo_id==10010&&ifwake==1)
  {
      const char* text ="收到指令，正在前往泛在感知展台"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到前往D点指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="D";
      pub.publish(msg_pub);
  }
  if(todo_id==10011&&ifwake==1)
  {
      const char* text ="收到指令，开始巡逻"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到开始巡逻指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="patrol";
      pub.publish(msg_pub);
  }
  if(todo_id==10012&&ifwake==1)
  {
      const char* text ="收到指令，结束巡逻"; //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
	    printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("收到结束巡逻指令\n");
      play_wav((char*)filename_move);
      ifwake=0;
      std_msgs::String msg_pub;
      msg_pub.data ="stoppatrol";
      pub.publish(msg_pub);
  }
}

void ttsCallback(const std_msgs::String::ConstPtr& msg)
{
      int ret; 
      ROS_INFO("llm re: [%s]", msg->data.c_str());
      const char* text = msg->data.c_str(); //合成文本
      ret =text_to_speech(text, filename_move, session_begin_params);
      if (MSP_SUCCESS != ret)
      {
      printf("text_to_speech failed, error code: %d.\n", ret);
      }
      printf("合成完毕\n");
      printf("生成大模型回复\n");
      play_wav((char*)filename_move);

}



int main(int argc, char* argv[])
{
	srand((int)time(0));	
	int         ret                  = MSP_SUCCESS;
	const char* login_params         = "appid = 62d8c0dc, work_dir = .";//登录参数,appid与msc库绑定,请勿随意改动
	/*
	* rdn:           合成音频数字发音方式
	* volume:        合成音频的音量
	* pitch:         合成音频的音调
	* speed:         合成音频对应的语速
	* voice_name:    合成发音人
	* sample_rate:   合成音频采样率
	* text_encoding: 合成文本编码格式
	*
	*/
	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://www.xfyun.cn注册获取
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code: %d.\n", ret);
	}
        
	//printf("%s\n",session_begin_params);
	//printf("%s\n",filename);
        //printf("%s\n",filename_move);
	printf("\n###########################################################################\n");
	printf("## 语音合成（Text To Speech，TTS）技术能够自动将任意文字实时转换为连续的 ##\n");
	printf("## 自然语音，是一种能够在任何时间、任何地点，向任何人提供语音信息服务的  ##\n");
	printf("## 高效便捷手段，非常符合信息时代海量数据、动态更新和个性化查询的需求。  ##\n");
	printf("###########################################################################\n\n");
  
 
	ros::init(argc, argv, "handsfree_offlinetalker");

	ros::NodeHandle n;
	
	pub = n.advertise<std_msgs::String>("/recognizer/output", 1000);

  ros::Subscriber sub = n.subscribe("order_todo", 1000, chatterCallback);

  ros::Subscriber chat = n.subscribe("response_msg", 1000, ttsCallback);//

  ros::spin();


	return 0;
}

