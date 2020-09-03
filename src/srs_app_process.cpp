#include "srs_app_process.hpp"

#include <stdlib.h>
#include <string.h>


#ifndef WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#endif



using namespace std;

std::string join_vector_string(std::vector<std::string>& urlVec, std::string delim)
{
	std::string text;
	int i = 0;
	for (auto url : urlVec)
	{
		text.append(url);
		i++;
		if (i < urlVec.size())
			text.append(delim);
	}
	return text;
}
bool srs_string_starts_with(string str, string flag)
{
	return str.find(flag) == 0;
}
string srs_string_trim_start(string str, string trim_chars)
{
	std::string ret = str;

	for (int i = 0; i < (int)trim_chars.length(); i++) {
		char ch = trim_chars.at(i);

		while (!ret.empty() && ret.at(0) == ch) {
			ret.erase(ret.begin());

			// ok, matched, should reset the search
			i = -1;
		}
	}

	return ret;
}

SrsProcess::SrsProcess()
{
    is_started         = false;
    fast_stopped       = false;
    pid                = -1;
}

SrsProcess::~SrsProcess()
{
}

int SrsProcess::get_pid()
{
    return pid;
}

bool SrsProcess::started()
{
    return is_started;
}

srs_error_t SrsProcess::initialize(string binary, vector<string> argv)
{
    srs_error_t err = srs_success;
    
    bin = binary;
    cli = "";
    
    params.clear();


	for (int i = 0; i < (int)argv.size(); i++) {
		std::string ffp = argv[i];
		std::string nffp = (i < (int)argv.size() - 1) ? argv[i + 1] : "";
		std::string nnffp = (i < (int)argv.size() - 2) ? argv[i + 2] : "";

		// >file
		if (srs_string_starts_with(ffp, ">")) {
			stdout_file = ffp.substr(1);
			continue;
		}

		// 1>file
		if (srs_string_starts_with(ffp, "1>")) {
			stdout_file = ffp.substr(2);
			continue;
		}

		// 2>file
		if (srs_string_starts_with(ffp, "2>")) {
			stderr_file = ffp.substr(2);
			continue;
		}

		// 1 >X
		if (ffp == "1" && srs_string_starts_with(nffp, ">")) {
			if (nffp == ">") {
				// 1 > file
				if (!nnffp.empty()) {
					stdout_file = nnffp;
					i++;
				}
			}
			else {
				// 1 >file
				stdout_file = srs_string_trim_start(nffp, ">");
			}
			// skip the >
			i++;
			continue;
		}

		// 2 >X
		if (ffp == "2" && srs_string_starts_with(nffp, ">")) {
			if (nffp == ">") {
				// 2 > file
				if (!nnffp.empty()) {
					stderr_file = nnffp;
					i++;
				}
			}
			else {
				// 2 >file
				stderr_file = srs_string_trim_start(nffp, ">");
			}
			// skip the >
			i++;
			continue;
		}

		params.push_back(ffp);
	}



	actual_cli = join_vector_string(params, " ");
	cli = join_vector_string(argv, " ");
    return err;
}

srs_error_t srs_redirect_output(string from_file, int to_fd)
{
    srs_error_t err = srs_success;
    
#ifndef  WIN32
    // use default output.
    if (from_file.empty()) {
        return err;
    }
    
    // redirect the fd to file.
    int fd = -1;
    int flags = O_CREAT|O_RDWR|O_APPEND;
    mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
    
    if ((fd = ::open(from_file.c_str(), flags, mode)) < 0) {
        srs_warn("open process %d %s failed", to_fd, from_file.c_str());
		return ERROR_FORK_OPEN_LOG;
    }
    
    if (dup2(fd, to_fd) < 0) {
		srs_warn("dup2 process %d failed", to_fd);
		return ERROR_FORK_DUP2_LOG;
    }
    
    ::close(fd);
#endif // ! WIN32

    
    return err;
}

srs_error_t SrsProcess::start()
{
	srs_error_t err = srs_success;

	if (is_started) {
		return err;
	}


	// generate the argv of process.
	srs_info("fork process: %s", cli.c_str());


#ifndef WIN32
	// for log
	int cid = 0;// _srs_context->get_id();
	int ppid = getpid();

	// TODO: fork or vfork?
	if ((pid = fork()) < 0) {
		srs_warn("vfork process failed, cli=%s", cli.c_str());
		return -1;
	}

	// for osx(lldb) to debug the child process.
	// user can use "lldb -p <pid>" to resume the parent or child process.
	//kill(0, SIGSTOP);

	// child process: ffmpeg encoder engine.
	if (pid == 0) {
		// ignore the SIGINT and SIGTERM
		signal(SIGINT, SIG_IGN);
		signal(SIGTERM, SIG_IGN);

		// for the stdout, ignore when not specified.
		// redirect stdout to file if possible.
		if ((err = srs_redirect_output(stdout_file, STDOUT_FILENO)) != srs_success) {
			srs_warn("redirect output. err:%d", err);
			return  err;
		}

		// for the stderr, ignore when not specified.
		// redirect stderr to file if possible.
		if ((err = srs_redirect_output(stderr_file, STDERR_FILENO)) != srs_success) {
			srs_warn("redirect output. err:%d", err);
			return  err;
		}

		// No stdin for process, @bug https://github.com/ossrs/srs/issues/1592
		if ((err = srs_redirect_output("/dev/null", STDIN_FILENO)) != srs_success) {
			srs_warn("redirect output. err:%d", err);
			return  err;
		}

		// should never close the fd 3+, for it myabe used.
		// for fd should close at exec, use fnctl to set it.

		// log basic info to stderr.
		if (true) {
			fprintf(stdout, "\n");
			fprintf(stdout, "process ppid=%d, cid=%d, pid=%d, in=%d, out=%d, err=%d\n",
				ppid, cid, getpid(), STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
			fprintf(stdout, "process binary=%s, cli: %s\n", bin.c_str(), cli.c_str());
			fprintf(stdout, "process actual cli: %s\n", actual_cli.c_str());
		}

		// memory leak in child process, it's ok.
		char** argv = new char*[params.size() + 1];
		for (int i = 0; i < (int)params.size(); i++) {
			std::string& p = params[i];

			// memory leak in child process, it's ok.
			char* v = new char[p.length() + 1];
			argv[i] = strcpy(v, p.data());
		}
		argv[params.size()] = NULL;

		// use execv to start the program.
		int r0 = execv(bin.c_str(), argv);
		if (r0 < 0) {
			srs_warn("fork process failed, errno=%d(%s)", errno, strerror(errno));
		}
		exit(r0);
	}

	// parent.
	if (pid > 0) {
		// Wait for a while for process to really started.
		// @see https://github.com/ossrs/srs/issues/1634#issuecomment-597568840
		srs_usleep(00 * SRS_UTIME_MILLISECONDS);

		int status = 0;
		pid_t p = waitpid(pid, &status, WNOHANG);
		
		//½ø³ÌÆô¶¯Ê§°Ü
		if ((WIFEXITED(status) && WEXITSTATUS(status) != 0)){
			printf("child process terminated. exit status is %d\n", WEXITSTATUS(status));
			return -1;
		}
		



        is_started = true;
        srs_trace("forked process, pid=%d, bin=%s, stdout=%s, stderr=%s, argv=%s",
                  pid, bin.c_str(), stdout_file.c_str(), stderr_file.c_str(), actual_cli.c_str());
        return err;
    }
    
#endif

    return err;
}

srs_error_t SrsProcess::cycle()
{
    srs_error_t err = srs_success;
    
    if (!is_started) {
        return err;
    }
    
    // ffmpeg is prepare to stop, donot cycle.
    if (fast_stopped) {
        return err;
    }
#ifndef WIN32
    int status = 0;
    pid_t p = waitpid(pid, &status, WNOHANG);
    
    if (p < 0) {
		srs_warn("process waitpid failed, pid=%d", pid);
		return ERROR_SYSTEM_WAITPID;
        
    }
    
    if (p == 0) {
        //srs_info("process process pid=%d is running.", pid);
        return err;
    }
    
    srs_trace("process pid=%d terminate, please restart it.", pid);
    is_started = false;
#endif
    return err;
}

void SrsProcess::stop()
{
    if (!is_started) {
        return;
    }
    
    // kill the ffmpeg,
    // when rewind, upstream will stop publish(unpublish),
    // unpublish event will stop all ffmpeg encoders,
    // then publish will start all ffmpeg encoders.
#ifndef WIN32 
    srs_error_t err = SrsUtil::srs_kill_forced(pid);
    if (err != srs_success) {
        srs_warn("ignore kill the process failed, pid=%d. err=", pid);
        return ;
    }
    
#endif
    // terminated, set started to false to stop the cycle.
    is_started = false;
}

void SrsProcess::fast_stop()
{
    int ret = 0;
    
    if (!is_started) {
        return;
    }
    
    if (pid <= 0) {
        return;
    }
#ifndef WIN32   
    if (kill(pid, SIGTERM) < 0) {
        ret = ERROR_SYSTEM_KILL;
        srs_warn("ignore fast stop process failed, pid=%d. ret=%d", pid, ret);
        return;
    }
#endif
    return;
}

void SrsProcess::fast_kill()
{
    int ret = 0;

#ifndef WIN32
    if (!is_started) {
        return;
    }

    if (pid <= 0) {
        return;
    }

    if (kill(pid, SIGKILL) < 0) {
        ret = ERROR_SYSTEM_KILL;
        srs_warn("ignore fast kill process failed, pid=%d. ret=%d", pid, ret);
        return;
    }

    // Try to wait pid to avoid zombie FFMEPG.
    int status = 0;
    waitpid(pid, &status, WNOHANG);
#endif
    return;
}

srs_error_t SrsUtil::srs_kill_forced(int& pid)
{
	srs_error_t err = srs_success;

	if (pid <= 0) {
		return err;
	}

#ifndef WIN32
	// first, try kill by SIGTERM.
	if (kill(pid, SIGTERM) < 0) {
		srs_warn("kill failed.");
		return ERROR_SYSTEM_KILL;
	}

	// wait to quit.
	srs_trace("send SIGTERM to pid=%d", pid);
	const int SRS_PROCESS_QUIT_TIMEOUT_MS = 1000;
	for (int i = 0; i < SRS_PROCESS_QUIT_TIMEOUT_MS / 10; i++) {
		int status = 0;
		pid_t qpid = -1;
		if ((qpid = waitpid(pid, &status, WNOHANG)) < 0) {
			srs_warn("kill failed.");
			return ERROR_SYSTEM_KILL;
		}

		// 0 is not quit yet.
		if (qpid == 0) {
			srs_usleep(10 * 1000);
			continue;
		}

		// killed, set pid to -1.
		srs_trace("SIGTERM stop process pid=%d ok.", pid);
		pid = -1;

		return err;
	}

	// then, try kill by SIGKILL.
	if (kill(pid, SIGKILL) < 0) {
		srs_warn("kill failed.");
		return ERROR_SYSTEM_KILL;
	}

	// wait for the process to quit.
	// for example, ffmpeg will gracefully quit if signal is:
	//         1) SIGHUP     2) SIGINT     3) SIGQUIT
	// other signals, directly exit(123), for example:
	//        9) SIGKILL    15) SIGTERM
	int status = 0;
	// @remark when we use SIGKILL to kill process, it must be killed,
	//      so we always wait it to quit by infinite loop.
	while (waitpid(pid, &status, 0) < 0) {
		srs_usleep(10 * 1000);
		continue;
	}

#endif
	srs_trace("SIGKILL stop process pid=%d ok.", pid);
	pid = -1;

	return err;
}