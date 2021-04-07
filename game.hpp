#pragma once
#include "utility.hpp"
#include <list>

// 隐藏光标
#define HIDE_CURSOR() printf("\033[?25l")
// 显示光标
#define SHOW_CURSOR() printf("\033[?25h")

#define CLOSE_ATTR  "\033[m"
#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define PURPLE      "\033[35m"
#define DEEP_GREEN  "\033[36m"
#define WHITE       "\033[37m"

#define MAP_W 64
#define MAP_H 32

enum MapType
{
    BLANK = 0,
    BORDER,
    SNAKE,
    FOOD
};

enum GameOptType
{
    MOVE_FORWARD = 0,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT
};

struct TGameFrame
{
    size_t      szFrameID;

    int         optType[2];
};

struct TGameCmd
{
    TGameCmd(){}
    TGameCmd(size_t gid, int cid, int type):
        szGameID(gid), nClientID(cid), optType(type){}

    size_t      szGameID;

    int         nClientID;

    int         optType;
};

class CMap
{
public:
    CMap();
    ~CMap();

    void init();

    void refresh();

    int* operator[](int i)
    {
        return m_map[i];
    }

    void inc_overlap(int x, int y)
    {
        ++ m_overlap[x][y];
    }

    void dec_overlap(int x, int y)
    {
        -- m_overlap[x][y];
    }

    int get_overlap(int x, int y)
    {
        return m_overlap[x][y];
    }

    void random_make_food();

private:
    std::string     m_strMapColor;

    std::string     m_strFoodColor;

    int             m_map[MAP_H][MAP_W];

    int             m_overlap[MAP_H][MAP_W];
};

class CSnakeNode
{
public:
    CSnakeNode(){}
    CSnakeNode(int x, int y): m_cox(x), m_coy(y){}
    ~CSnakeNode(){}

    int m_cox;

    int m_coy;
};

class CSnake
{
public:
    CSnake();
    CSnake(CMap* pmap, std::string color = RED);
    ~CSnake();

public:

    void init();

    void move_up();

    void move_down();

    void move_left();

    void move_right();

    void move_forward();

    void set_color(std::string color)
    {
        m_strColor = color;
    }

    std::string get_color()
    {
        return m_strColor;
    }

    void add_node(CSnakeNode node)
    {
        m_snake.push_back(node);
    }

private:
    void move_core(int r_x, int r_y); //参数为相对移动距离

private:
    std::list<CSnakeNode>   m_snake;

    CMap*           m_pMap;

    std::string     m_strColor;

    std::mutex      m_mt;
};

class CGame
{
    typedef int G_ClientID; //客户端
    typedef int G_RoomOwner;//房主
    typedef std::map<G_ClientID, std::shared_ptr<CTaskQueue<int>>> G_GameOptMap;
    typedef std::map<G_ClientID, int> G_GameReadyMap;

public:
    CGame(std::string strName);
    ~CGame();

    void start(int port);

    void over();

    bool add_client(G_ClientID client);

    bool remove_client(G_ClientID client);

    bool ready(G_ClientID client);

    bool quit_ready(G_ClientID client);

    bool get_ready_status(G_ClientID client);

    bool is_all_ready();

    int get_client_nums();

    void get_client_ids(std::vector<int>& vecCids);

    std::string get_name(){ return m_strRoomName; }

    int set_room_owner();

    int set_room_owner(G_ClientID cid);

    int get_room_owner(){ return m_roomOwner; }

    void add_gameopt(G_ClientID client, GameOptType type);

private:
    void send_frame_thread_func(int port);

private:

    std::string     m_strRoomName;

    G_GameOptMap    m_mapGameOpt;

    G_GameReadyMap  m_mapGameReady;

    std::mutex      m_mtx;

    bool            m_bExitSendFrame;

    G_RoomOwner     m_roomOwner;
};

class CGameServer
{
    typedef uint64_t G_GameID;
    typedef std::map<G_GameID, std::shared_ptr<CGame>> G_GameMap;

public:
    CGameServer();
    ~CGameServer();

    uint64_t create_game(int cid, std::string& strGameName);

    void remove_game(G_GameID gid);

    void remove_player(int cid);

    void remove_player(G_GameID gid, int cid);

    int add_game_player(G_GameID gid, int cid);

    int game_ready(G_GameID gid, int cid);

    int quit_game_ready(G_GameID gid, int cid);

    void game_start(G_GameID gid);

    void game_over(G_GameID gid);

    bool get_game_ready_status(G_GameID gid);

    std::shared_ptr<CGame> get_game(G_GameID gid);

    std::string get_gameid_list();

    void get_cid_list(G_GameID id, std::vector<int>& vecCids);

    int get_room_owner(G_GameID id);

    int get_player_nums(G_GameID id);

private:

    bool is_game_name_existed(std::string& strGameName);

    G_GameMap   m_mapGame;

    CSnowFlake  m_cSnowFlake;

    std::mutex  m_mtx;
};

class CGameClient
{
    typedef int G_GameID;
    typedef int G_ClientID;
    typedef CTaskQueue<std::shared_ptr<TGameFrame>> GameFrameQue;
    typedef CTaskQueue<std::shared_ptr<TGameCmd>>   GameCmdQue;
public:
    CGameClient();
    ~CGameClient();

    void start(int port);

    void set_random_seed(int seed)
    {
        m_nRandSeed = seed;
    }

    void random_make_snake();

private:
    void init();

    void recv_frame_thread_func(int port);

    void refresh_thread_func();

private:
    CMap            m_map;

    std::map<G_ClientID, CSnake>    m_mapSnake;

    GameCmdQue      m_queGameCmd;

    GameFrameQue    m_queGameFrame;

    bool            m_bExitRecvFrame;

    bool            m_bExitRefresh;

    G_GameID        m_gameID;

    G_ClientID      m_clientID;

    int             m_nRandSeed;
};




