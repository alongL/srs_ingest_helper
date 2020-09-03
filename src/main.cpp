
#include <cstdio>
#include <iostream>

#include "httplib.h"
#include "json.hpp"


#include "csv.hpp"
#include "timer.hpp"
#include "proxytaskmgr.h"




using namespace httplib;
using namespace std;




#ifndef WIN32
#include <signal.h>
void signalDeal(int sig)
{
	//ctrl+c, pkill
	if (sig == SIGINT || sig == SIGTERM)    
	{
		ProxytaskMgr::getinstance().fast_kill();
		printf("receive signal %d, and kill sub process.\n", sig);
		exit(0);
	}
	else
	{
		printf("receive signal %d. but ignore.\n", sig);
	}
}

#endif





string log(const Request &req, const Response &res) {
	string s;
	char buf[BUFSIZ];

	snprintf(buf, sizeof(buf), "%s %s %s\n", req.method.c_str(),
		req.version.c_str(), req.path.c_str());
	s += buf;

	return s;
}


static int timer_cnt = 0;
void check()
{
	timer_cnt++;
	if(timer_cnt % 3 == 0)
		ProxytaskMgr::getinstance().check(timer_cnt);

}


const string CSV_FILE = "tasks.csv";
map<string,string> load_task_from_csv()
{
	map<string, string> taskmap;
	try {
		csv::CSVFormat format;
		format.trim({ ' ', '\t' });
		csv::CSVReader reader(CSV_FILE, format);
		csv::CSVRow row;
		while (reader.read_row(row))
		{
			if (!row["dest"].is_null() &&
				!row["src"].is_null()
				)
			{
				auto dest = row["dest"].get();
				auto src = row["src"].get();

				taskmap[src] = dest;
			}
		}
	}
	catch (exception& ex)
	{
		printf("exception:%s\n", ex.what());
	}
	return taskmap;
}






int main(int argc, const char **argv)
{

#ifndef WIN32
	signal(SIGINT, signalDeal);   //注册SIGINT对应的处理函数
	signal(SIGTERM, signalDeal);  //注册SIGTERM对应的处理函数
#endif

	httplib::Server svr;

	int ret = ProxytaskMgr::getinstance().init();
	if (ret != 0)
	{
		printf("srs start failed.\n");
		exit(-1);
		return ret;
	}




	svr.set_error_handler([](const Request & /*req*/, Response &res) {
		const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
		char buf[BUFSIZ];
		snprintf(buf, sizeof(buf), fmt, res.status);
		res.set_content(buf, "text/html");
	});

	svr.set_logger(
		[](const Request &req, const Response &res) { cout << log(req, res); });

	auto port = 8086;
	if (argc > 1) 
	{ port = atoi(argv[1]); }


	svr.Get("/hi", [](const Request& req, Response& res) {
		res.set_content("Hello World!", "text/plain");
	});

	svr.Get("/", [](const Request& req, Response& res) {
		res.set_redirect("/liststream");
		res.set_content("Hello World!", "text/plain");
	});

	svr.Get("/addstream", [](const Request& req, Response& res) {

		int ret = 0;
		string message;
		string hls_path ;
		auto src = req.get_param_value("src");
		auto dest = req.get_param_value("dest");
		if (src.empty() || dest.empty())
		{
			ret = 1;
			message = "param error.";
			goto end;
		}
		
		ret = ProxytaskMgr::getinstance().add_task(src, dest);
		message = ProxytaskMgr::getinstance().get_errmsg();
		hls_path = ProxytaskMgr::getinstance().get_hls_path(dest);

	end:
		nlohmann::json j3;
		j3["result"] = ret;
		j3["msg"] = message;
		j3["hls_path"] = hls_path;
		j3["src"] = src;
		j3["dest"] = dest;


		printf("addstream. result:%d, msg:%s. src:%s, dest:%s\n", ret, message.c_str(), src.c_str(), dest.c_str());

		res.set_content(j3.dump(), "application/json");
	});
	svr.Get("/delstream", [](const Request& req, Response& res) {

		auto dest = req.get_param_value("dest");
		int ret = 0;
		string message = "sucess";
		string hls_path = "";
		auto ptask = ProxytaskMgr::getinstance().get_task_by_dest(dest);
		if (ptask)
		{
			hls_path = ptask->hls;
			ret = ProxytaskMgr::getinstance().del_task(dest);
		}
		message = ProxytaskMgr::getinstance().get_errmsg();

		nlohmann::json j3;
		j3["result"] = ret;
		j3["msg"] = message;
		j3["hls_path"] = hls_path;

		printf("delstream. result:%d, msg:%s, dest:%s\n", ret, message.c_str(), dest.c_str());
		
		
		res.set_content(j3.dump(), "application/json");
	});
	svr.Get("/liststream", [](const Request& req, Response& res) {
		const auto & taskMap = ProxytaskMgr::getinstance().get_task_list();

		auto jarray = nlohmann::json::array();
		for (auto &item : taskMap)
		{
			nlohmann::json jtemp;
			auto ptask = item.second;
			if (!ptask)
				continue;

			jtemp["src"] = ptask->src;
			jtemp["dest"] = ptask->dest;
			jtemp["hls_path"] = ptask->hls;

			jarray.emplace_back(jtemp);
		}
		
		nlohmann::json j3;
		j3["result"] = 0;
		j3["msg"] = "sucess";
		j3["cnt"] = jarray.size();
		j3["media"] = jarray;

	
		res.set_content(j3.dump(), "application/json");
	});

	

	map<string,string> taskmap = load_task_from_csv();
	for (auto item : taskmap)
	{
		auto src = item.first;
		auto dest = item.second;
		ProxytaskMgr::getinstance().add_task(src, dest);
	}


	Timer m_timer;
	m_timer.StartTimer(1000, std::bind(check));//check the ffmpeg process;



	cout << "The server started at port " << port  << endl;
	svr.listen("0.0.0.0", port);

	return 0;
}
