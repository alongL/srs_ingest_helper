#include "proxytaskmgr.h"
#include "srs_app_process.hpp"

#include "csv.hpp"

#ifndef WIN32
#include<sys/stat.h>  
#endif


using namespace std;
const std::string CSV_FILE = "tasks.csv";




IngestTask::~IngestTask()
{
	free(ffmpeg);
}


int IngestTask::start()
{
	return ffmpeg->start();
}

void IngestTask::stop()
{
	ffmpeg->stop();
}

srs_error_t IngestTask::cycle()
{
	return ffmpeg->cycle();
}

void IngestTask::fast_stop()
{
	ffmpeg->fast_stop();
}

void IngestTask::fast_kill()
{
	ffmpeg->fast_kill();
}

std::string replaceAll(const std::string& instr, const std::string& from, const std::string& to) {
	if (from.empty())
		return instr;
	std::string str = instr;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return str;
}
void IngestTask::init(std::string src, std::string dest)
{
	this->src = src;
	this->dest = dest;
	std::string rtmp_port = ProxytaskMgr::getinstance().get_rtmp_port();
	std::string hls_port = ProxytaskMgr::getinstance().get_hls_port();
	this->rtmp = "rtmp://127.0.0.1:" + rtmp_port + dest;					        //rtmp://localhost/live/my
	string http_host = "http://127.0.0.1:";
	this->hls = http_host + hls_port + dest + std::string("/hls.m3u8");   //http://localhost/live/my/hls.m3u8
	
	string ffmpeg_path = "./bin/ffmpeg";
	string cmd = "chmod +x " + ffmpeg_path;
	system(cmd.c_str());

	ffmpeg = new SrsFFMPEG(ffmpeg_path);
	auto name = replaceAll(dest, "/", "_");
	string log_file = "./logs/ffmpeg" + name + ".log";// 日志文件./logs/ffmpeg_live_my.log
	
	ffmpeg->initialize(src, rtmp, log_file);
}





//////////////////////////////////////////////////////////////////////////////////////

int ProxytaskMgr::init()
{
	

	m_srs_process = new SrsProcess();
	string srs_path = "./bin/srs";
	std::vector<string> argVec;
	argVec.push_back(srs_path);
	argVec.push_back("-c");
	argVec.push_back("./bin/srs.conf");
	m_srs_process->initialize(srs_path, argVec);


#ifndef WIN32
	string cmd = "chmod +x " + srs_path;
	system(cmd.c_str());
	system("mkdir -p logs");
	system("pkill -9  -x srs ");
	system("pkill -9  -x ffmpeg ");
	system("sleep 0.1");
#endif


	// start srs
	if (0 != m_srs_process->start())
	{
		printf("srs start failed.\n");
		return -1;
	}
	else
		printf("srs start sucess.\n");


	load_from_db();

	return 0;
}






//内部调用
IngestTask* ProxytaskMgr::insert_task(std::string src, std::string dest)
{
	//目前没有此任务，添加
	IngestTask* ptask = new IngestTask();
	ptask->init(src, dest);
	m_taskMap[dest] = ptask;
	return ptask;
}


//外部api调用
int ProxytaskMgr::add_task(std::string src, std::string dest)
{
	//如果已经存在，不添加
	if (m_taskMap.find(dest) != m_taskMap.end())
	{
		m_errmsg = "dest: " + dest + " exists.";
		return 0;
	}

	
	auto ptask = insert_task(src, dest);
	if (ptask)
		ptask->start();
	
	save_to_db();

	m_errmsg = "sucess";
	return 0;
}



//删除某个任务
int ProxytaskMgr::del_task(std::string dest)
{
	//如果已经存在，不添加
	auto iter = m_taskMap.find(dest);
	if (iter == m_taskMap.end())
	{
		m_errmsg = "no this dest." + dest;
		return 0;
	}

	auto ptask = iter->second;
	ptask->stop();
	m_taskMap.erase(iter);
	delete ptask;


	save_to_db();
	m_errmsg = "sucess";
	return 0;
}


int ProxytaskMgr::del_task_by_src(std::string src)
{
	vector<string> delVec;

	for (auto &item : m_taskMap)
	{
		auto &dest = item.first;
		auto ptask = item.second;
		if (ptask &&ptask->src == src)
		{
			delVec.push_back(dest);
		}
	}
	
	

	for (auto &dest : delVec)
	{
		del_task(dest);//save_to_db in del_task()
	}

	return 0;
}


IngestTask * ProxytaskMgr::get_task_by_src(const std::string & src)
{
	IngestTask *out_ptask = nullptr;

	for (auto &item : m_taskMap)
	{
		auto &dest = item.first;
		auto ptask = item.second;
		if (ptask &&ptask->src == src)
		{
			out_ptask = ptask;
			break;
		}
	}

	return out_ptask;
}

IngestTask * ProxytaskMgr::get_task_by_dest(const std::string & dest)
{
	auto iter = m_taskMap.find(dest);
	if (iter == m_taskMap.end())
		return nullptr;
	else
		return iter->second;
}


std::string ProxytaskMgr::get_hls_path(const std::string &dest)
{
	auto iter = m_taskMap.find(dest);
	if (iter == m_taskMap.end())
		return "";

	auto ptask = iter->second;
	if (ptask)
		return ptask->hls;
	else
		return "";
}


//第一次强制加载配置文件
void ProxytaskMgr::loadConfig(bool force)
{
	m_hls_port = "8081";
	m_rtmp_port = "1936";


	printf("m_hls_port: %s\n", m_hls_port.c_str());
	printf("m_rtmp_port: %s\n", m_rtmp_port.c_str());
}

//结束子进程运行
void ProxytaskMgr::fast_kill()
{
	//ffmpeg
	for (auto item : m_taskMap) 
	{
		auto ptask = item.second;
		if (ptask)
			ptask->fast_kill();
	}
	//srs
	if (m_srs_process)
	{
		m_srs_process->fast_kill();
	}

}


int ProxytaskMgr::check(int timecnt)
{
	//检查srs是否正在运行
	if (m_srs_process)
	{
		m_srs_process->cycle();//检查
		m_srs_process->start();//启动。内部有标志，如果未在运行才运行程序
	}


	for (auto &item : m_taskMap)
	{	
		int err = 0;
		auto task = item.second;
		
		// check ffmpeg status.
		if ((err = task->cycle()) != srs_success) {
			printf("ingest cycle. err:%d\n", err);
		}
		
		if ((err = task->start()) != srs_success) {
			printf("ingester start. err:%d\n", err);
		}
	}

	if (timecnt % 10 == 0)//every 10 seconds
	{
		loadConfig(); //读取加载配置文件
	}

	return 0;
}




inline bool is_file(const std::string &path)
{
#ifndef S_ISREG
#define S_ISREG(m) (((m)&S_IFREG) == S_IFREG)
#endif // S_ISREG

	struct stat st;
	return stat(path.c_str(), &st) >= 0 && S_ISREG(st.st_mode);
}


int ProxytaskMgr::load_from_db()
{
	if (!is_file(CSV_FILE))
	{
		return 0;
	}

	try {
		csv::CSVFormat format;
		format.trim({ ' ', '\t' });
		csv::CSVReader reader(CSV_FILE, format);
		
		auto nameVec = reader.get_col_names();
		csv::CSVRow row;
		while (reader.read_row(row)) 
		{
			if (! row["dest"].is_null() &&
				! row["src"].is_null()
				)
			{
				auto dest = row["dest"].get();
				auto src = row["src"].get();


				auto ptask = insert_task(src, dest);
				if (ptask)
					ptask->start();
			}
			
		}

		printf("ProxytaskMgr::load_from_db(). size:%d\n", m_taskMap.size());
		for (auto item : m_taskMap)
		{
			string dest = item.first;
			auto ptask = item.second;
			if (ptask)
			{
				auto src = ptask->src;
				printf("src:%s, dest:%s\n", src.c_str(),dest.c_str());
			}
		}

	}
	catch (exception& ex)
	{
		printf("exception:%s\n", ex.what());
	}

	return 0;
}


int ProxytaskMgr::save_to_db()
{
	stringstream ss; // Can also use ifstream, etc.
	auto writer = csv::make_csv_writer(ss);
	writer << vector<string>({ "dest", "src" });
	for (auto item : m_taskMap)
	{
		string dest = item.first;
		auto ptask = item.second;
		if (ptask)
		{
			auto src = ptask->src;
			writer << vector<string>({ dest, src });
		}
	}

	try {
		fstream file(CSV_FILE, ios::binary | ios::out);
		file << ss.str();
		file.close();
	}
	catch (exception& ex)
	{
		printf("save_to_db exception:%s\n", ex.what());
	}
	return 0;
}





