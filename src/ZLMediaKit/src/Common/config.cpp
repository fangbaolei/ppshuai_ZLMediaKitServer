﻿/*
 * MIT License
 *
 * Copyright (c) 2016 xiongziliang <771730766@qq.com>
 *
 * This file is part of ZLMediaKit(https://github.com/xiongziliang/ZLMediaKit).
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Util/NoticeCenter.h>
#include "Common/config.h"
#include "Util/util.h"
#include "Util/onceToken.h"
#include "Network/sockutil.h"

using namespace toolkit;

namespace mediakit {

bool loadIniConfig(const char *ini_path){
    string ini;
    if(ini_path){
        ini = ini_path;
    }else{
        ini = exePath() + ".ini";
    }
	try{
        mINI::Instance().parseFile(ini);
        NoticeCenter::Instance().emitEvent(Broadcast::kBroadcastReloadConfig);
        return true;
	}catch (std::exception &ex) {
        mINI::Instance().dumpFile(ini);
        return false;
	}
}
////////////广播名称///////////
namespace Broadcast {
const char kBroadcastMediaChanged[] = "kBroadcastMediaChanged";
const char kBroadcastRecordMP4[] = "kBroadcastRecordMP4";
const char kBroadcastHttpRequest[] = "kBroadcastHttpRequest";
const char kBroadcastOnGetRtspRealm[] = "kBroadcastOnGetRtspRealm";
const char kBroadcastOnRtspAuth[] = "kBroadcastOnRtspAuth";
const char kBroadcastMediaPlayed[] = "kBroadcastMediaPlayed";
const char kBroadcastMediaPublish[] = "kBroadcastMediaPublish";
const char kBroadcastFlowReport[] = "kBroadcastFlowReport";
const char kBroadcastReloadConfig[] = "kBroadcastReloadConfig";
const char kBroadcastShellLogin[] = "kBroadcastShellLogin";
const char kBroadcastNotFoundStream[] = "kBroadcastNotFoundStream";

const char kFlowThreshold[] = "broadcast.flowThreshold";

onceToken token([](){
    mINI::Instance()[kFlowThreshold] = 1024;
},nullptr);
} //namespace Broadcast

////////////HTTP配置///////////
namespace Http {
#define HTTP_FIELD "http."

//http 文件发送缓存大小
#define HTTP_SEND_BUF_SIZE (64 * 1024)
const char kSendBufSize[] = HTTP_FIELD"sendBufSize";

//http 最大请求字节数
#define HTTP_MAX_REQ_SIZE (4*1024)
const char kMaxReqSize[] = HTTP_FIELD"maxReqSize";

//http keep-alive秒数
#define HTTP_KEEP_ALIVE_SECOND 10
const char kKeepAliveSecond[] = HTTP_FIELD"keepAliveSecond";

//http keep-alive最大请求数
#define HTTP_MAX_REQ_CNT 100
const char kMaxReqCount[] = HTTP_FIELD"maxReqCount";

//http 字符编码
#if defined(_WIN32)
#define HTTP_CHAR_SET "gb2312"
#else
#define HTTP_CHAR_SET "utf-8"
#endif
const char kCharSet[] = HTTP_FIELD"charSet";

//http 服务器根目录
#define HTTP_ROOT_PATH (exeDir() + "httpRoot")
const char kRootPath[] = HTTP_FIELD"rootPath";

//http 404错误提示内容
#define HTTP_NOT_FOUND "<html>"\
		"<head><title>404 Not Found</title></head>"\
		"<body bgcolor=\"white\">"\
		"<center><h1>您访问的资源不存在！</h1></center>"\
		"<hr><center>"\
		 SERVER_NAME\
		"</center>"\
		"</body>"\
		"</html>"
const char kNotFound[] = HTTP_FIELD"notFound";


onceToken token([](){
	mINI::Instance()[kSendBufSize] = HTTP_SEND_BUF_SIZE;
	mINI::Instance()[kMaxReqSize] = HTTP_MAX_REQ_SIZE;
	mINI::Instance()[kKeepAliveSecond] = HTTP_KEEP_ALIVE_SECOND;
	mINI::Instance()[kMaxReqCount] = HTTP_MAX_REQ_CNT;
	mINI::Instance()[kCharSet] = HTTP_CHAR_SET;
	mINI::Instance()[kRootPath] = HTTP_ROOT_PATH;
	mINI::Instance()[kNotFound] = HTTP_NOT_FOUND;
},nullptr);

}//namespace Http

////////////SHELL配置///////////
namespace Shell {
#define SHELL_FIELD "shell."

#define SHELL_MAX_REQ_SIZE 1024
const char kMaxReqSize[] = SHELL_FIELD"maxReqSize";

onceToken token([](){
	mINI::Instance()[kMaxReqSize] = SHELL_MAX_REQ_SIZE;
},nullptr);
} //namespace Shell

////////////RTSP服务器配置///////////
namespace Rtsp {
#define RTSP_FIELD "rtsp."
const char kAuthBasic[] = RTSP_FIELD"authBasic";

onceToken token([](){
	//默认Md5方式认证
	mINI::Instance()[kAuthBasic] = 0;
},nullptr);

} //namespace Rtsp

////////////RTMP服务器配置///////////
namespace Rtmp {
#define RTMP_FIELD "rtmp."
const char kModifyStamp[] = RTMP_FIELD"modifyStamp";

onceToken token([](){
	mINI::Instance()[kModifyStamp] = true;
},nullptr);
} //namespace RTMP


////////////RTP配置///////////
namespace Rtp {
#define RTP_FIELD "rtp."

//RTP打包最大MTU,公网情况下更小
#define RTP_VIDOE_MTU_SIZE 1400
const char kVideoMtuSize[] = RTP_FIELD"videoMtuSize";

#define RTP_Audio_MTU_SIZE 600
const char kAudioMtuSize[] = RTP_FIELD"audioMtuSize";

//RTP排序缓存最大个数
#define RTP_MAX_RTP_COUNT 50
const char kMaxRtpCount[] = RTP_FIELD"maxRtpCount";

//如果RTP序列正确次数累计达到该数字就启动清空排序缓存
#define RTP_CLEAR_COUNT 10
const char kClearCount[] = RTP_FIELD"clearCount";

//最大RTP时间为13个小时，每13小时回环一次
#define RTP_CYCLE_MS (13*60*60*1000)
const char kCycleMS[] = RTP_FIELD"cycleMS";


onceToken token([](){
	mINI::Instance()[kVideoMtuSize] = RTP_VIDOE_MTU_SIZE;
	mINI::Instance()[kAudioMtuSize] = RTP_Audio_MTU_SIZE;
	mINI::Instance()[kMaxRtpCount] = RTP_MAX_RTP_COUNT;
	mINI::Instance()[kClearCount] = RTP_CLEAR_COUNT;
	mINI::Instance()[kCycleMS] = RTP_CYCLE_MS;
},nullptr);
} //namespace Rtsp

////////////组播配置///////////
namespace MultiCast {
#define MULTI_FIELD "multicast."
//组播分配起始地址
const char kAddrMin[] = MULTI_FIELD"addrMin";
//组播分配截止地址
const char kAddrMax[] = MULTI_FIELD"addrMax";
//组播TTL
#define MULTI_UDP_TTL 64
const char kUdpTTL[] = MULTI_FIELD"udpTTL";

onceToken token([](){
	mINI::Instance()[kAddrMin] = "239.0.0.0";
	mINI::Instance()[kAddrMax] = "239.255.255.255";
	mINI::Instance()[kUdpTTL] = MULTI_UDP_TTL;
},nullptr);

} //namespace MultiCast

////////////录像配置///////////
namespace Record {
#define RECORD_FIELD "record."

//查看录像的应用名称
#define RECORD_APP_NAME "record"
const char kAppName[] = RECORD_FIELD"appName";

//每次流化MP4文件的时长,单位毫秒
#define RECORD_SAMPLE_MS 100
const char kSampleMS[] = RECORD_FIELD"sampleMS";

//MP4文件录制大小,默认一个小时
#define RECORD_FILE_SECOND (60*60)
const char kFileSecond[] = RECORD_FIELD"fileSecond";

//录制文件路径
#define RECORD_FILE_PATH HTTP_ROOT_PATH
const char kFilePath[] = RECORD_FIELD"filePath";

onceToken token([](){
	mINI::Instance()[kAppName] = RECORD_APP_NAME;
	mINI::Instance()[kSampleMS] = RECORD_SAMPLE_MS;
	mINI::Instance()[kFileSecond] = RECORD_FILE_SECOND;
	mINI::Instance()[kFilePath] = RECORD_FILE_PATH;
},nullptr);

} //namespace Record

////////////HLS相关配置///////////
namespace Hls {
#define HLS_FIELD "hls."

//HLS切片时长,单位秒
#define HLS_SEGMENT_DURATION 3
const char kSegmentDuration[] = HLS_FIELD"segDur";

//HLS切片个数
#define HLS_SEGMENT_NUM 3
const char kSegmentNum[] = HLS_FIELD"segNum";

//HLS文件写缓存大小
#define HLS_FILE_BUF_SIZE (64 * 1024)
const char kFileBufSize[] = HLS_FIELD"fileBufSize";

//录制文件路径
#define HLS_FILE_PATH (HTTP_ROOT_PATH)
const char kFilePath[] = HLS_FIELD"filePath";

onceToken token([](){
	mINI::Instance()[kSegmentDuration] = HLS_SEGMENT_DURATION;
	mINI::Instance()[kSegmentNum] = HLS_SEGMENT_NUM;
	mINI::Instance()[kFileBufSize] = HLS_FILE_BUF_SIZE;
	mINI::Instance()[kFilePath] = HLS_FILE_PATH;
},nullptr);

} //namespace Hls


namespace Client {
const char kNetAdapter[] = "net_adapter";
const char kRtpType[] = "rtp_type";
const char kRtspUser[] = "rtsp_user" ;
const char kRtspPwd[] = "rtsp_pwd";
const char kRtspPwdIsMD5[] = "rtsp_pwd_md5";
const char kTimeoutMS[] = "protocol_timeout_ms";
const char kMediaTimeoutMS[] = "media_timeout_ms";
const char kBeatIntervalMS[] = "beat_interval_ms";
const char kMaxAnalysisMS[] = "max_analysis_ms";

}

}  // namespace mediakit


