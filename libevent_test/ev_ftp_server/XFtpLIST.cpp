#include "XFtpLIST.h"
#include <iostream>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <string>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif
using namespace std;

void XFtpLIST::Write(struct bufferevent *bev)
{
	//4 226 Transfer complete�������
	ResCMD("226 Transfer complete\r\n");
	//5 �ر�����
	Close();

}
void XFtpLIST::Event(struct bufferevent *bev, short what)
{
	//����Է�����ϵ������߻��������п����ղ���BEV_EVENT_EOF����
	if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		cout << "XFtpLIST BEV_EVENT_EOF | BEV_EVENT_ERROR |BEV_EVENT_TIMEOUT" << endl;
		Close();
	}
	else if(what&BEV_EVENT_CONNECTED)
	{
		cout << "XFtpLIST BEV_EVENT_CONNECTED" << endl;
	}
}
//����Э��
void XFtpLIST::Parse(std::string type, std::string msg)
{
	string resmsg = "";

	if (type == "PWD")
	{
		//257 "/" is current directory.
		resmsg = "257 \"";
		resmsg += cmdTask->curDir;
		resmsg += "\" is current dir.\r\n";

		ResCMD(resmsg);
	}
	else if (type == "LIST")
	{
		//1��������ͨ�� 2 150 3 ����Ŀ¼����ͨ�� 4 �������226 5 �ر�����
		//����ͨ���ظ���Ϣ ʹ������ͨ������Ŀ¼
		//-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n
		//1 ��������ͨ�� 
		ConnectPORT();
		//2 1502 150
		ResCMD("150 Here comes the directory listing.\r\n");
		//string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n";
        std::string listdata = GetListData(cmdTask->rootDir + cmdTask->curDir);
		//3 ����ͨ������
		Send(listdata);
	}
    else if (type == "CWD")//�л�Ŀ¼
    {
        //ȡ�������е�·��
        //CWD test\r\n
        int pos = msg.rfind(" ") + 1;
        //ȥ����β��\r\n
        std::string path = msg.substr(pos, msg.size() - pos - 2);
        if (path[0] == '/')
        {
            cmdTask->curDir = path;
        }
        else
        {
            if (cmdTask->curDir[cmdTask->curDir.size() - 1] != '/')
            {
                cmdTask->curDir += "/";
            }
            cmdTask->curDir += path + "/";
        }
        ResCMD("250 Directory changed success.\r\n");
    }
    else if (type == "CDUP")//�ص��ϲ�Ŀ¼
    {
        // /Debug/test /Debug /Debug/
        std::string cur_path = cmdTask->curDir;
        if (cur_path == "/")
        {
            ResCMD("250 Directory changed success.\r\n");
        }

        //ͳһȥ����β�� /
        if (cur_path[cur_path.size() - 1] == '/')
        {
            cur_path = cur_path.substr(0, cur_path.size() - 1);
        }

        // /Debug/test.xxx /Debug 
        int pos = cur_path.rfind("/");
        cur_path = cur_path.substr(0, pos);
        cmdTask->curDir = cur_path;
        ResCMD("250 Directory changed success.\r\n");
    }
}

std::string XFtpLIST::GetListData(std::string path)
{
    //string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n";
    std::string data = "";
#ifdef _WIN32
    //�洢�ļ���Ϣ
    _finddata_t file;
    //Ŀ¼������
    path += "/*.*";
    intptr_t dir = _findfirst(path.c_str(), $file);
    if (dir < 0)
    {
        return data;
    }

	do
    {
        std::string tmp;
		//�Ƿ���Ŀ¼ȥ�� . �� ..
		if (file.attrib & _A_SUBDIR)
        {
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
            {
                continue;
            }
            tmp = "drwxrwxrwx 1 root group ";
        }
        else
        {
            tmp = "-rwxrwxrwx 1 root group ";
        }

        //�ļ���С
        char buf[1024];
        sprintf(buf, "%u", file.size);
        tmp += buf;

        //����ʱ��
        strftime(buf, sizeof(buf) - 1, "%b %d %H:%M ", localtime(file.time_write));
        tmp += buf;
        tmp += file.name;
        data += tmp;
        data += "\r\n";

    }while (_findNext(dir, $file) == 0);

    
#else
    DIR* dir;
    struct dirent* ptr;

    if ((dir = opendir(path.c_str())) == NULL)
	{
        perror("Open dir error...");
		return data;
    }
 
    while ((ptr = readdir(dir)) != NULL)
    {
        std::string temp;
		if (strcmp(ptr->d_name, ".")==0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == DT_REG)    ///file
        {
            temp = "-rwxrwxrwx 1 root group ";
        }
        else if(ptr->d_type == DT_LNK)    ///link file
        {
            temp = "lrwxrwxrwx 1 root group ";
        }
		else if(ptr->d_type == DT_DIR)    ///dir
        {
            temp = "drwxrwxrwx 1 root group ";
        }
        //�ļ���С
        struct stat st;
        std::string file_path = path + "/" + ptr->d_name;
        int res = stat(file_path.c_str(), &st);
        if (res < 0)
        {
            std::string error("error open file ");
            error += file_path;
            perror(error.c_str());
            return data;
        }

        char buf[1024];
        sprintf(buf, "%ld", st.st_size);
        temp += buf;

        //����ʱ��
        strftime(buf, sizeof(buf) - 1, "%b %d %H:%M ", localtime(&st.st_mtime));
        temp += buf;
        temp += ptr->d_name;
        data += temp;
        data += "\r\n";
    }
    closedir(dir);
#endif

    return data;
}
