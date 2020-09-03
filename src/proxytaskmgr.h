#pragma once

#include <string>
#include <map>
#include <string>
#include <vector>

#include "srs_common.h"
#include "srs_app_process.hpp"
#include "srs_app_ffmpeg.hpp"



class IngestTask
{
public:
	virtual ~IngestTask();
	int start();
	void stop();
	int cycle();
	void fast_stop();
	void fast_kill();

	void init(std::string src, std::string dest);


	std::string src;
	std::string dest;// 类似于/live/my
	std::string rtmp;// rtmp://127.0.0.1:1936/live/my
	std::string hls;// http://127.0.0.1:8081/live/my.m3u8
	bool enable=true;

	int fail_cnt=0;
	time_t starttime=time(0);
	SrsFFMPEG* ffmpeg=nullptr;
};




class ProxytaskMgr
{
private:
	ProxytaskMgr() {}
	std::map<std::string, IngestTask*> m_taskMap;
	SrsProcess* m_srs_process = nullptr;
	std::string  m_errmsg;

	//这两个端口号在srs中配置
	std::string m_hls_port = "8081";
	std::string m_rtmp_port = "1936";
public:
	static ProxytaskMgr& getinstance()
	{
		static ProxytaskMgr instance;
		return instance;
	}
	~ProxytaskMgr() { delete m_srs_process; }
	int init();
	int check(int timecnt);

	std::string get_hls_port() { return m_hls_port; }
	std::string get_rtmp_port() { return m_rtmp_port; }

private:
	IngestTask* insert_task(std::string src, std::string dest);
	void loadConfig(bool force=false);

	int load_from_db();
	int save_to_db();

public:
	//添加任务
	int add_task(std::string src, std::string dest);
	//删除某个任务
	int del_task(std::string dest);
	int del_task_by_src(std::string src);

	const decltype(m_taskMap)&  get_task_list()const{ 	return m_taskMap; }

	IngestTask* get_task_by_src(const std::string &src);
	IngestTask* get_task_by_dest(const std::string &dest);

	//获取对应的hls地址
	std::string get_hls_path(const std::string &dest);
	std::string get_errmsg() {	return m_errmsg; }

	void fast_kill();
};