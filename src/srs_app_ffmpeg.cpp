
#include "srs_app_ffmpeg.hpp"
#include "srs_app_process.hpp"

#include <stdlib.h>

#ifndef WIN32
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif


#include <vector>
using namespace std;




SrsFFMPEG::SrsFFMPEG(std::string ffmpeg_bin)
{
    ffmpeg = ffmpeg_bin;
    process = new SrsProcess();
}

SrsFFMPEG::~SrsFFMPEG()
{
    stop();
    
    free(process);
}

void SrsFFMPEG::append_iparam(string iparam)
{
    iparams.push_back(iparam);
}


string SrsFFMPEG::output()
{
    return _output;
}

srs_error_t SrsFFMPEG::initialize(string in, string out, string log)
{
    srs_error_t err = srs_success;
    
    _input = in;
    _output = out;
    log_file = log;
    
    return err;
}



srs_error_t SrsFFMPEG::start()
{
    srs_error_t err = srs_success;
    
    if (process->started()) {
        return err;
    }
    
    // the argv for process.
    params.clear();
    
    // argv[0], set to ffmpeg bin.
    // The  execv()  and  execvp() functions ....
    // The first argument, by convention, should point to
    // the filename associated  with  the file being executed.
    params.push_back(ffmpeg);
    
	params.push_back("-loglevel");
	params.push_back("warning");

	// ./ffmpeg -loglevel info  -f flv -i rtmp://yunyuntx.com/live/tv 
	// -vcodec copy -acodec copy -f flv -y rtmp://127.0.0.1:1936/live/tv
	string input = "-f flv -i";
	params.push_back("-f");
	params.push_back("flv");
	params.push_back("-i");
	

	//string rtmpUrl = "http://yunyuntx.com/live/my";
	params.push_back(_input);

	string output = "-vcodec copy -acodec copy -f flv -y";
	params.push_back("-vcodec");
	params.push_back("copy");
	params.push_back("-acodec");
	params.push_back("copy");
	params.push_back("-f");
	params.push_back("flv");
	params.push_back("-y");
	//params.push_back(output);
	
	string destUrl = "http://127.0.0.1:1935/live/my";
	params.push_back(_output);

	// when specified the log file.
	if (!log_file.empty()) {
		// stdout
		params.push_back("1");
		params.push_back(">");
		params.push_back(log_file);
		// stderr
		params.push_back("2");
		params.push_back(">");
		params.push_back(log_file);
	}
    
    // initialize the process.
    if ((err = process->initialize(ffmpeg, params)) != srs_success) {
        return srs_error_wrap(err, "init process");
    }
    
    return process->start();
}

srs_error_t SrsFFMPEG::cycle()
{
    return process->cycle();
}

void SrsFFMPEG::stop()
{
    process->stop();
}

void SrsFFMPEG::fast_stop()
{
    process->fast_stop();
}

void SrsFFMPEG::fast_kill()
{
    process->fast_kill();
}



