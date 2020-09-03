

#ifndef SRS_COMMON_H
#define SRS_COMMON_H


#ifndef WIN32
#include <unistd.h>

#define srs_usleep usleep
#else
#include <Windows.h>

#define srs_usleep Sleep
#endif

#include <time.h>
#include <string>

static time_t srs_get_system_time()
{
	return time(0);
}

typedef int  srs_error_t;
using srs_utime_t = time_t;
typedef int pid_t;

enum srs_error {
	srs_success = 0
};


#define logprintf( fmt, ...)  do{\
	printf(fmt,##__VA_ARGS__);\
	printf("\n");\
}\
while(0)



#define srs_info   logprintf
#define srs_warn   logprintf  
#define srs_trace  logprintf  


// The time unit in ms, for example 100 * SRS_UTIME_MILLISECONDS means 100ms.
#define SRS_UTIME_MILLISECONDS 1000
// The time unit in ms, for example 120 * SRS_UTIME_SECONDS means 120s.
#define SRS_UTIME_SECONDS 1000000LL


static int Fun_error(int err, std::string msg)
{
	printf("err:%d, msg:%s\n", err, msg.c_str());
	return err; 
}


#define srs_error_wrap(err, str) Fun_error(err,str)











#define ERROR_ENCODER_FORK                  3028
#define ERROR_FORK_OPEN_LOG                 3030
#define ERROR_FORK_DUP2_LOG                 3031

///////////////////////////////////////////////////////
// The system error.
///////////////////////////////////////////////////////
#define ERROR_SOCKET_CREATE                 1000
#define ERROR_SOCKET_SETREUSE               1001
#define ERROR_SOCKET_BIND                   1002
#define ERROR_SOCKET_LISTEN                 1003
#define ERROR_SOCKET_CLOSED                 1004
#define ERROR_SOCKET_GET_PEER_NAME          1005
#define ERROR_SOCKET_GET_PEER_IP            1006
#define ERROR_SOCKET_READ                   1007
#define ERROR_SOCKET_READ_FULLY             1008
#define ERROR_SOCKET_WRITE                  1009
#define ERROR_SOCKET_WAIT                   1010
#define ERROR_SOCKET_TIMEOUT                1011
#define ERROR_SOCKET_CONNECT                1012
#define ERROR_ST_SET_EPOLL                  1013
#define ERROR_ST_INITIALIZE                 1014
#define ERROR_ST_OPEN_SOCKET                1015
#define ERROR_ST_CREATE_LISTEN_THREAD       1016
#define ERROR_ST_CREATE_CYCLE_THREAD        1017
#define ERROR_ST_CONNECT                    1018
#define ERROR_SYSTEM_PACKET_INVALID         1019
#define ERROR_SYSTEM_CLIENT_INVALID         1020
#define ERROR_SYSTEM_ASSERT_FAILED          1021
#define ERROR_READER_BUFFER_OVERFLOW        1022
#define ERROR_SYSTEM_CONFIG_INVALID         1023
#define ERROR_SYSTEM_CONFIG_DIRECTIVE       1024
#define ERROR_SYSTEM_CONFIG_BLOCK_START     1025
#define ERROR_SYSTEM_CONFIG_BLOCK_END       1026
#define ERROR_SYSTEM_CONFIG_EOF             1027
#define ERROR_SYSTEM_STREAM_BUSY            1028
#define ERROR_SYSTEM_IP_INVALID             1029
#define ERROR_SYSTEM_FORWARD_LOOP           1030
#define ERROR_SYSTEM_WAITPID                1031
#define ERROR_SYSTEM_BANDWIDTH_KEY          1032
#define ERROR_SYSTEM_BANDWIDTH_DENIED       1033
#define ERROR_SYSTEM_PID_ACQUIRE            1034
#define ERROR_SYSTEM_PID_ALREADY_RUNNING    1035
#define ERROR_SYSTEM_PID_LOCK               1036
#define ERROR_SYSTEM_PID_TRUNCATE_FILE      1037
#define ERROR_SYSTEM_PID_WRITE_FILE         1038
#define ERROR_SYSTEM_PID_GET_FILE_INFO      1039
#define ERROR_SYSTEM_PID_SET_FILE_INFO      1040
#define ERROR_SYSTEM_FILE_ALREADY_OPENED    1041
#define ERROR_SYSTEM_FILE_OPENE             1042
//#define ERROR_SYSTEM_FILE_CLOSE             1043
#define ERROR_SYSTEM_FILE_READ              1044
#define ERROR_SYSTEM_FILE_WRITE             1045
#define ERROR_SYSTEM_FILE_EOF               1046
#define ERROR_SYSTEM_FILE_RENAME            1047
#define ERROR_SYSTEM_CREATE_PIPE            1048
#define ERROR_SYSTEM_FILE_SEEK              1049
#define ERROR_SYSTEM_IO_INVALID             1050
#define ERROR_ST_EXCEED_THREADS             1051
#define ERROR_SYSTEM_SECURITY               1052
#define ERROR_SYSTEM_SECURITY_DENY          1053
#define ERROR_SYSTEM_SECURITY_ALLOW         1054
#define ERROR_SYSTEM_TIME                   1055
#define ERROR_SYSTEM_DIR_EXISTS             1056
#define ERROR_SYSTEM_CREATE_DIR             1057
#define ERROR_SYSTEM_KILL                   1058
#define ERROR_SYSTEM_CONFIG_PERSISTENCE     1059
#define ERROR_SYSTEM_CONFIG_RAW             1060
#define ERROR_SYSTEM_CONFIG_RAW_DISABLED    1061
#define ERROR_SYSTEM_CONFIG_RAW_NOT_ALLOWED 1062
#define ERROR_SYSTEM_CONFIG_RAW_PARAMS      1063
#define ERROR_SYSTEM_FILE_NOT_EXISTS        1064
#define ERROR_SYSTEM_HOURGLASS_RESOLUTION   1065
#define ERROR_SYSTEM_DNS_RESOLVE            1066
#define ERROR_SYSTEM_FRAGMENT_UNLINK        1067
#define ERROR_SYSTEM_FRAGMENT_RENAME        1068
#define ERROR_THREAD_DISPOSED               1069
#define ERROR_THREAD_INTERRUPED             1070
#define ERROR_THREAD_TERMINATED             1071
#define ERROR_THREAD_DUMMY                  1072
#define ERROR_ASPROCESS_PPID                1073
#define ERROR_EXCEED_CONNECTIONS            1074
#define ERROR_SOCKET_SETKEEPALIVE           1075
#define ERROR_SOCKET_NO_NODELAY             1076
#define ERROR_SOCKET_SNDBUF                 1077
#define ERROR_THREAD_STARTED                1078
#define ERROR_SOCKET_SETREUSEADDR           1079
#define ERROR_SOCKET_SETCLOSEEXEC           1080
#define ERROR_SOCKET_ACCEPT                 1081



#endif

