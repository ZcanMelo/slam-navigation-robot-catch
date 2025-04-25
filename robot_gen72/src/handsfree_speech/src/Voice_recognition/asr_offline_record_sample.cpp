/*
* 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int8.h"
#include<ros/package.h>
#include "../../include/qisr.h"
#include "../../include/msp_cmn.h"
#include "../../include/msp_errors.h"
#include "../../include/speech_recognizer.h"

#include <iostream>

#define FRAME_LEN	640 
#define	BUFFER_SIZE	4096
#define SAMPLE_RATE_16K     (16000)
#define SAMPLE_RATE_8K      (8000)
#define MAX_GRAMMARID_LEN   (32)
#define MAX_PARAMS_LEN      (1024)

static const std::string s_filename_res = "fo|"+ros::package::getPath("handsfree_speech")+"/cfg/msc/res/asr/common.jet";
static const std::string s_filename_build = ros::package::getPath("handsfree_speech")+"/cfg/msc/res/asr/GrmBuilld";
static const std::string s_filename_file = ros::package::getPath("handsfree_speech")+"/cfg/msc/res/asr/talking.bnf";
static std::string s_asr_audiof =ros::package::getPath("handsfree_speech")+"/res/speaking_ok.wav";
const char * ASR_RES_PATH        = s_filename_res.c_str(); //离线语法识别资源路径
const char * GRM_BUILD_PATH      = s_filename_build.c_str(); //构建离线语法识别网络生成数据保存路径
const char * GRM_FILE            = s_filename_file.c_str(); //构建离线识别语法网络所用的语法文件

int rest_flag;


typedef struct _UserData {
	int     build_fini; //标识语法构建是否完成
	int     update_fini; //标识更新词典是否完成
	int     errcode; //记录语法构建或更新词典回调错误码
	char    grammar_id[MAX_GRAMMARID_LEN]; //保存语法构建返回的语法ID
}UserData;

static UserData asr_data;

int build_grammar(UserData *udata); //构建离线识别语法网络

int run_asr(UserData *udata); //进行离线语法识别

ros::Publisher pub;
ros::Publisher llm;

int build_grm_cb(int ecode, const char *info, void *udata)
{
	UserData *grm_data = (UserData *)udata;

	if (NULL != grm_data) {
		grm_data->build_fini = 1;
		grm_data->errcode = ecode;
	}

	if (MSP_SUCCESS == ecode && NULL != info) {
		printf("构建语法成功！ 语法ID:%s\n", info);
		if (NULL != grm_data)
			snprintf(grm_data->grammar_id, MAX_GRAMMARID_LEN - 1, info);
	}
	else
		printf("构建语法失败！%d\n", ecode);

	return 0;
}

int build_grammar(UserData *udata)
{
	FILE *grm_file                           = NULL;
	char *grm_content                        = NULL;
	unsigned int grm_cnt_len                 = 0;
	char grm_build_params[MAX_PARAMS_LEN]    = {NULL};
	int ret                                  = 0;

	grm_file = fopen(GRM_FILE, "rb");	
	if(NULL == grm_file) {
		printf("打开\"%s\"文件失败！[%s]\n", GRM_FILE, strerror(errno));
		return -1; 
	}

	fseek(grm_file, 0, SEEK_END);
	grm_cnt_len = ftell(grm_file);
	fseek(grm_file, 0, SEEK_SET);

	grm_content = (char *)malloc(grm_cnt_len + 1);
	if (NULL == grm_content)
	{
		printf("内存分配失败!\n");
		fclose(grm_file);
		grm_file = NULL;
		return -1;
	}
	fread((void*)grm_content, 1, grm_cnt_len, grm_file);
	grm_content[grm_cnt_len] = '\0';
	fclose(grm_file);
	grm_file = NULL;

	snprintf(grm_build_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH
		);
	ret = QISRBuildGrammar("bnf", grm_content, grm_cnt_len, grm_build_params, build_grm_cb, udata);

	free(grm_content);
	grm_content = NULL;
	return ret;
}

static int16_t get_order(char *_xml_result){
  if(_xml_result == NULL){
    printf("no");
  }
//get confidence
  //printf("\n%s",_xml_result);
  char *str_con_first1 = strstr(_xml_result,"<confidence>");
  //printf("\n%s",str_con_first1);
  char *str_con_second1 = strstr(str_con_first1,"</confidence");
  //printf("\n%s",str_con_second1);
   char *str_con_first2 = strstr(str_con_second1,"<confidence>");
  //printf("\n%s",str_con_first1);
  char *str_con_second2 = strstr(str_con_first2,"</confidence");
  //printf("\n%s",str_con_second1);
  char str_confidence2[10] = {};
  strncpy(str_confidence2, str_con_first2+12, str_con_second2 - str_con_first2-12);
  //printf("\n%s\n",str_confidence2);
  char *str_con_first = strstr(_xml_result,"<object>");
  char *str_con_second = strstr(str_con_first,"</object");
  char str_confidence[30] = {};
  char str_order[5] = {};
  strncpy(str_confidence, str_con_first+25, 5);
  //memcpy(str_order,str_confidence,5);
  //printf("\n%s\n",str_order);
  std_msgs::String msg_pub;
  //printf("%s\n",s_filename_res.c_str());
  //printf("%s\n",s_filename_build.c_str());
  //printf("%s\n",s_filename_file.c_str());
  if(atoi(str_confidence2) <40)
   {
		msg_pub.data="00001";
		pub.publish(msg_pub);
	 
		if(rest_flag == 0)
		{
			//置信度小于40,发送给大预言模型
		std_msgs::Int8 llm_flag;
		llm_flag.data = 1;
		llm.publish(llm_flag);
		}
	 
   }
   else
   {
  		msg_pub.data =str_confidence;
  		pub.publish(msg_pub);
  		//如果收到休眠信息，不发送llm_flag
	  	if(msg_pub.data == "55555")
	  	{	
	  		rest_flag= 1;
	  	}
	  if(msg_pub.data == "99999")
	  	{	
	  		rest_flag= 0;
	  	}

   }
}

static void show_result(char *string, char is_over)
{
	//printf("\rResult: [ %s ]", string);
	get_order(string);
	if(is_over)
		putchar('\n');
}

static char *g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;


void on_result(const char *result, char is_last)
{
	if (result) {
		size_t left = g_buffersize - 1 - strlen(g_result);
		size_t size = strlen(result);
		if (left < size) {
			g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
			if (g_result)
				g_buffersize += BUFFER_SIZE;
			else {
				printf("mem alloc failed\n");
				return;
			}
		}
		strncat(g_result, result, size);
		show_result(g_result, is_last);
	}
}
void on_speech_begin()
{
	if (g_result)
	{
		free(g_result);
	}
	g_result = (char*)malloc(BUFFER_SIZE);
	g_buffersize = BUFFER_SIZE;
	memset(g_result, 0, g_buffersize);

	printf("Start Listening...\n");
}
void on_speech_end(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
		printf("\nSpeaking done \n");
	else
		printf("\nRecognizer error %d\n", reason);
}

/* demo send audio data from a file */
static void demo_file(const char* audio_file, const char* session_begin_params)
{
	int	errcode = 0;
	FILE*	f_pcm = NULL;
	char*	p_pcm = NULL;
	unsigned long	pcm_count = 0;
	unsigned long	pcm_size = 0;
	unsigned long	read_size = 0;
	struct speech_rec iat;
	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	if (NULL == audio_file)
		goto iat_exit;

	f_pcm = fopen(audio_file, "rb");
	if (NULL == f_pcm)
	{
		printf("\nopen [%s] failed! \n", audio_file);
		goto iat_exit;
	}

	fseek(f_pcm, 0, SEEK_END);
	pcm_size = ftell(f_pcm);
	fseek(f_pcm, 0, SEEK_SET);

	p_pcm = (char *)malloc(pcm_size);
	if (NULL == p_pcm)
	{
		printf("\nout of memory! \n");
		goto iat_exit;
	}

	read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm);
	if (read_size != pcm_size)
	{
		printf("\nread [%s] error!\n", audio_file);
		goto iat_exit;
	}

	errcode = sr_init(&iat, session_begin_params, SR_USER, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed : %d\n", errcode);
		goto iat_exit;
	}

	errcode = sr_start_listening(&iat);
	if (errcode) {
		printf("\nsr_start_listening failed! error code:%d\n", errcode);
		goto iat_exit;
	}

	while (1)
	{
		unsigned int len = 10 * FRAME_LEN; /* 200ms audio */
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		ret = sr_write_audio_data(&iat, &p_pcm[pcm_count], len);

		if (0 != ret)
		{
			printf("\nwrite audio data failed! error code:%d\n", ret);
			goto iat_exit;
		}

		pcm_count += (long)len;
		pcm_size -= (long)len;		
	}

	errcode = sr_stop_listening(&iat);
	if (errcode) {
		printf("\nsr_stop_listening failed! error code:%d \n", errcode);
		goto iat_exit;
	}

iat_exit:
	if (NULL != f_pcm)
	{
		fclose(f_pcm);
		f_pcm = NULL;
	}
	if (NULL != p_pcm)
	{
		free(p_pcm);
		p_pcm = NULL;
	}

	sr_stop_listening(&iat);
	sr_uninit(&iat);
}

/* demo recognize the audio from microphone */
static void demo_mic(const char* session_begin_params)
{
	int errcode;
	int i = 0;

	struct speech_rec iat;

	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	errcode = sr_init(&iat, session_begin_params, SR_MIC, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed\n");
		return;
	}
	errcode = sr_start_listening(&iat);
	if (errcode) {
		printf("start listen failed %d\n", errcode);
	}
	/* demo 15 seconds recording */
	while(i++ < 15)
		sleep(1);
	errcode = sr_stop_listening(&iat);
	if (errcode) {
		printf("stop listening failed %d\n", errcode);
	}

	sr_uninit(&iat);
}

int run_asr(UserData *udata)
{
	char asr_params[MAX_PARAMS_LEN]    = {NULL};
	const char *rec_rslt               = NULL;
	const char *session_id             = NULL;
	const char *asr_audiof             = NULL;
	FILE *f_pcm                        = NULL;
	char *pcm_data                     = NULL;
	long pcm_count                     = 0;
	long pcm_size                      = 0;
	int last_audio                     = 0;

	int aud_stat                       = MSP_AUDIO_SAMPLE_CONTINUE;
	int ep_status                      = MSP_EP_LOOKING_FOR_SPEECH;
	int rec_status                     = MSP_REC_STATUS_INCOMPLETE;
	int rss_status                     = MSP_REC_STATUS_INCOMPLETE;
	int errcode                        = -1;
	int aud_src                        = 0;
	//离线语法识别参数设置
	snprintf(asr_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, local_grammar = %s, \
		result_type = xml, result_encoding = UTF-8, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH,
		udata->grammar_id
		);
	asr_audiof =s_asr_audiof.c_str();
	demo_file(asr_audiof, asr_params);
	//demo_mic(asr_params);
	return 0;
}

void chatterCallback(const std_msgs::String::ConstPtr& msg)
{
  int ret;
  //ROS_INFO("I heard: [%s]", msg->data.c_str());
  std_msgs::String msg_pub;
  msg_pub.data ="00000";
  pub.publish(msg_pub);
  //sleep(5);
  ret = run_asr(&asr_data);
}

int main(int argc, char* argv[])
{
	const char *login_config    = "appid = 62d8c0dc, work_dir = ."; //登录参数
	int ret                     = 0 ;
	char c;

	ret = MSPLogin(NULL, NULL, login_config); //第一个参数为用户名，第二个参数为密码，传NULL即可，第三个参数是登录参数
	if (MSP_SUCCESS != ret) {
		printf("登录失败：%d\n", ret);

	}

	memset(&asr_data, 0, sizeof(UserData));
	printf("构建离线识别语法网络...\n");
        ret = build_grammar(&asr_data);
        
        ros::init(argc, argv, "handsfree_offlinelistener");

	ros::NodeHandle n;

	pub = n.advertise<std_msgs::String>("order_todo", 1000);	

	llm = n.advertise<std_msgs::Int8>("record_end_flag", 1000);	

	ros::Subscriber sub = n.subscribe("vad_listener_hear", 1000, chatterCallback);

	ros::spin();

	return 0;

	
}
