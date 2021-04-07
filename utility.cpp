#include "utility.hpp"
#include <iostream>
#include <sstream>

#include <stdlib.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>                                                                              
#include <sys/stat.h>

#include <string.h>
#include <dirent.h>                                                                                 
#include <unistd.h>
#include <fcntl.h>
#include <iconv.h>

using std::cin;
using std::cout;

CSemaphore::CSemaphore(){}

CSemaphore::~CSemaphore(){}

void CSemaphore::Wait()
{
    std::unique_lock<std::mutex> lck(m_mtx);
    m_cv.wait(lck);
}

int CSemaphore::WaitFor(int ms)
{
    std::unique_lock<std::mutex> lck(m_mtx);
    std::cv_status res = m_cv.wait_for(lck, std::chrono::milliseconds(ms));
    if(res == std::cv_status::timeout)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void CSemaphore::NoticeOne()
{
    std::unique_lock<std::mutex> lck(m_mtx);
    m_cv.notify_one();
}

void CSemaphore::NoticeAll()
{
    std::unique_lock<std::mutex> lck(m_mtx);
    m_cv.notify_all();
}

CSnowFlake::CSnowFlake()
{
    m_nMachineId = 0;
    m_nMaxSerialNum = (1 << 12) - 1;
    uint64_t curr_ms_ts = get_ms_ts();
    m_nKey = 0;
    m_nKey |= curr_ms_ts << 22;
    m_nKey |= m_nMachineId << 12;
    m_nLast_ms_ts = curr_ms_ts;
}

CSnowFlake::CSnowFlake(uint64_t machineid)
{
    //机器id大小范围（0~1023）
    m_nMachineId = machineid;
    m_nMaxSerialNum = (1 << 12) - 1;
    uint64_t curr_ms_ts = get_ms_ts();
    m_nKey = 0;
    m_nKey |= curr_ms_ts << 22;
    m_nKey |= m_nMachineId << 12;
    m_nLast_ms_ts = curr_ms_ts;
}

CSnowFlake::~CSnowFlake(){}

uint64_t CSnowFlake::get_sid()
{
    std::lock_guard<std::mutex> lck(m_mtx);
    uint64_t curr_ms_ts = get_ms_ts();
    if( curr_ms_ts > m_nLast_ms_ts )
    {
        m_nKey = 0;
        m_nKey |= curr_ms_ts << 22;
        m_nKey |= m_nMachineId << 12;
        m_nLast_ms_ts = curr_ms_ts;
        return m_nKey;
    }
    else
    {
        uint64_t nCurrSerialNum = m_nKey & m_nMaxSerialNum;
        //1毫秒内并发数超过序列号上限
        if( (nCurrSerialNum + 1) > m_nMaxSerialNum )
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            lck.~lock_guard();
            return get_sid();
        }
        else
        {
            //更新当前序列号
            m_nKey &= (~(m_nMaxSerialNum));
            m_nKey |= nCurrSerialNum + 1;
            return m_nKey;
        }
    }
}

uint64_t CSnowFlake::get_ms_ts()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ms_ts = tv.tv_sec + tv.tv_usec / 1000;
    return ms_ts;
}

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock, F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock, F_SETFL, opts)<0)
    {
        perror("fcntl(sock, SETFL, opts)");
        exit(1);
    }
}

void setreuseaddr(int sock)
{
    int opt = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    if(res < 0)
    {
        perror("setsockopt");
        exit(1);
    }
}


int readn(int fd, void *vptr, int n)
{
    int nleft;
    int nread;
    char *ptr;
 
    ptr = (char*)vptr;
    nleft = n;
 
    while(nleft > 0)
    {
        nread = read(fd, ptr, nleft);
        if(nread < 0)
        {
            if(errno == EINTR)
                nread = 0;      /* and call read() again */
            else
                return -1;
        }
        else if(nread == 0)
        {
            break;  /*  EOF */
        }
        
        nleft -= nread;
        ptr   += nread;
    }
    return (n - nleft);
}
 
int writen(int fd, const void *vptr, int n)
{
    int nleft;
    int nwritten;
    const char *ptr;
 
    ptr = (const char *)vptr;
    nleft = n;
 
    while(nleft > 0)
    {
        nwritten = write(fd, ptr, nleft);
        if(nwritten <= 0)
        {
            if(nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
 
        nleft -= nwritten;
        ptr   += nwritten;
    }
 
    return n;
}

void enter_any_key_to_continue()
{
    cout << "输入任意键继续...";
    get_input_string();
}

int get_input_number()
{
    int nInput = 0;
    while (1)
    {
        cin >> nInput;
        if (!cin.good())
        {
            cout << "输入错误，请重新输入: ";
            cin.clear(); //清空输入流状态位
            //cin.clear(cin.rdstate() & ~cin.failbit & ~cin.badbit & !cin.goodbit & !cin.eofbit);
            cin.ignore(128, '\n');
        }
        else
        {
            break;
        }
    }

    return nInput;
}

std::string get_input_string()
{
    std::string strInput;
    while (1)
    {
        cin >> strInput;
        if (!cin.good())
        {
            cout << "输入错误，请重新输入: ";
            cin.clear(); //清空输入流状态位
            //cin.clear(cin.rdstate() & ~cin.failbit & ~cin.badbit & !cin.goodbit & !cin.eofbit);
            cin.ignore(128, '\n');
        }
        else
        {
            break;
        }
    }

    return strInput;
}

static int Convert(const char* from_type, const char* to_type,
    const char* src, int src_length, char* dest, int dest_length)
{
    char* in = const_cast<char*>(src);
    char* out = dest;
    size_t insize = src_length;
    size_t outsize = dest_length;
    iconv_t conv = iconv_open(to_type, from_type);
    if (conv == (iconv_t)-1)
    {
        return -1;
    }

    size_t ret = iconv(conv, &in, &insize, &out, &outsize);
    if (ret == (size_t)-1)
    {
        iconv_close(conv);
        return -1;
    }

    iconv_close(conv);
    return dest_length - outsize;
}

//to_code//IGNORE  to_code//TRANSLIT
void TransCoding(const char* from_code, const char* to_code, const std::string& in, std::string& out)
{
    if (in.empty())
    {
        return;
    }
    
    int in_size = in.size() + 1;
    char* new_out = new char[2 * in_size];
    Convert(from_code, to_code, in.c_str(), in_size, new_out, 2 * in_size);
    out.assign(new_out);
    delete[] new_out;
}

void SplitStr(const std::string& source, const std::string& delimiter, std::vector<std::string>& result)
{
    char * strc = new char[strlen(source.c_str())+1];                                                     
    strcpy(strc, source.c_str());                                                                         
    char* tmpStr = strtok(strc, delimiter.c_str());                                                      
    while (tmpStr != NULL)                                                                             
    {                                                                                                  
        result.push_back(std::string(tmpStr));                                                      
        tmpStr = strtok(NULL, delimiter.c_str());                                                        
    }                                                                                                  
                                                                                                       
    delete[] strc;
}

std::string joinstr(const std::vector<std::string>& src, const std::string& conn_str)
{
    std::string ret;
    std::vector<std::string>::const_iterator citer = src.begin();
    for (; citer != src.end(); ++citer) {
        ret += *citer + conn_str;
    }
    //移除最后的连接字符 conn_str
    if (!src.empty()) {
        ret.erase(ret.end() - conn_str.size());
    }
    return ret;
}

std::string to_upper(const std::string& src)
{
    std::string str_ret(src);
    std::transform(src.begin(), src.end(), str_ret.begin(), [](const std::string::value_type& ch) {
                   return std::toupper(ch);
                   });
    return str_ret;
}

std::string to_lower(const std::string& src)
{
    std::string str_ret(src);
    std::transform(src.begin(), src.end(), str_ret.begin(), [](const std::string::value_type& ch) {
                   return std::tolower(ch);
                   });
    return str_ret;
}

bool start_with(const std::string& src, const std::string& start)
{
    std::string::size_type pos = src.find(start);
    if (pos == 0) {
        return true;
    }
    return false;
}

bool end_with(const std::string& src, const std::string& end)
{
    std::string::size_type pos = src.rfind(end);
    if (pos == (src.size() - end.size())) {
        return true;
    }
    return false;
}

static bool is_dir(const std::string& str_path) {                                                          
    struct stat st;                                                                                 
    if(stat(str_path.c_str(), &st) < 0) {                                                           
        fprintf(stderr, "invalid path %s\n", str_path.c_str());                                     
        return false;                                                                               
    }                                                                                               
    return S_ISDIR(st.st_mode);                                                                     
} 

void traverse_dir(const std::string& str_dir, std::vector<std::string>* vec_files, std::vector<std::string>* vec_dirs) {
    struct dirent *pdirent = NULL;                                                                  
    DIR *d_info = opendir(str_dir.c_str());                                                         
    if (!d_info) {                                                                                  
        fprintf(stderr, "can not open dir %s\n", str_dir.c_str());                                  
        return;                                                                                     
    }                                                                                               
    while ((pdirent = readdir(d_info)) != NULL) {                                                   
        if (strncmp(pdirent->d_name, ".", 1) == 0 ||                                                
                strncmp(pdirent->d_name, "..", 2) == 0) {                                           
            continue;                                                                               
        }                                                                                           
        if (pdirent->d_type == DT_UNKNOWN) {                                                        
            //未知类型（可能d_type不支持)                                                           
            std::ostringstream ostr;                                                                   
            ostr << str_dir << "/" << pdirent->d_name;                                                 
            if (is_dir(ostr.str())) {                                                                  
                if (vec_dirs) {                                                                        
                    vec_dirs->emplace_back(pdirent->d_name);                                           
                }                                                                                      
            } else {                                                                                   
                if (vec_files) {                                                                       
                    vec_files->emplace_back(pdirent->d_name);                                          
                }                                                                                   
            }                                                                                       
        } else if (pdirent->d_type == DT_REG) {                                                     
            //常规文件                                                                              
            if (vec_files) {                                                                        
                vec_files->emplace_back(pdirent->d_name);                                           
            }                                                                                       
        } else if (pdirent->d_type == DT_DIR) {                                                     
            //目录文件                                                                              
            if (vec_dirs) {                                                                         
                vec_dirs->emplace_back(pdirent->d_name);                                            
            }                                                                                       
        }                                                                                           
    }                                                                                               
    closedir(d_info);                                                                               
} 
