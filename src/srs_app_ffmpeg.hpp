#pragma once

#include "srs_common.h"



#include <vector>
#include <string>

class SrsConfDirective;
class SrsPithyPrint;
class SrsProcess;


class SrsFFMPEG
{
private:
    SrsProcess* process;
    std::vector<std::string> params;
    std::string log_file;
    
	std::string                 ffmpeg;
    std::vector<std::string>    iparams;
    std::vector<std::string>    perfile;
    std::string                 iformat;
    std::string                 _input;
    std::string                 _output;
public:
    SrsFFMPEG(std::string ffmpeg_bin);
    virtual ~SrsFFMPEG();
    
	virtual void append_iparam(std::string iparam);
    virtual std::string output();
    virtual srs_error_t initialize(std::string in, std::string out, std::string log);
    
	virtual srs_error_t start();
    virtual srs_error_t cycle();
    virtual void stop();
    virtual void fast_stop();
    virtual void fast_kill();
};




