#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <iostream>
#include <algorithm>

struct problem
{
    int status; // positive if ac, record total submision times
    int freeze_count; // record submision times during the freeze
    std::vector<int> freeze_status;
};

struct submission
{
    char problem_name;
    std::string status;
    int time;
};

struct team
{
    std::string name;
    int rank;
    int ac_count;
    int ac_time[26];
    int penalty_time;
    std::vector<submission> history;
    problem problems[26];
    std::set<int> freeze_problems;
    bool freeze; 
};

std::unordered_map<std::string, team> map; // to quickly find a team via its name

struct key_team
{
    std::string name;
    friend bool operator<(const key_team& A, const key_team& B)
    {
        team *a = &map[A.name], *b = &map[B.name];
        if (a->ac_count != b->ac_count) return a->ac_count > b->ac_count;
        if (a->penalty_time != b->penalty_time) return a->penalty_time < b->penalty_time;
        for (int i = a->ac_count - 1; i >= 0; i--)
            if (a->ac_time[i] != b->ac_time[i]) return a->ac_time[i] < b->ac_time[i];
        return a->name < b->name;
    }
};

bool freeze;
int problem_count;
std::string trash, teamname;
team tmpteam;
submission tmpsub;
problem tmppro;
key_team tmpkey;
std::string ranking[10010];
std::set<key_team> set; // keep track of "actual" ranking
std::set<key_team> freeze_teams; // keep track of freezed teams

void addteam();
void start();
void submit();
void flush();
void Freeze();
void scroll();
void query_submission();
void query_ranking();
void update(const std::string& name, const int& number, const submission& tmp);
void printlist();
void unfreeze(const std::string& name, const int& number, const std::string& nextteam);

int main()
{
    //freopen("data/bigger.in", "r", stdin);
    //freopen("ans.txt", "w", stdout);
    std::ios::sync_with_stdio(false);
    std::string ins;
    while(true)
    {
        std::cin >> ins;
        if (ins == "ADDTEAM") addteam();
        else if (ins == "START") break;
    }
    start();
    while(true)
    {
        std::cin >> ins;
        if (ins == "SUBMIT") submit();
        else if (ins == "QUERY_RANKING") query_ranking();
        else if (ins == "QUERY_SUBMISSION") query_submission();
        else if (ins == "FLUSH") 
        {
            flush();
            std::cout << "[Info]Flush scoreboard.\n";
        }
        else if (ins == "FREEZE") Freeze();
        else if (ins == "SCROLL") scroll();
        else if (ins == "ADDTEAM")
        {
            std::cout << "[Error]Add failed: competition has started.\n";
            std::getline(std::cin, trash);
        }
        else if (ins == "START")
        {
            std::cout << "[Error]Start failed: competition has started.\n";
            std::getline(std::cin, trash);
        }
        else if (ins == "END") break;
    }
    std::cout << "[Info]Competition ends.\n";
}

void addteam()
{
    std::cin >> teamname;
    if (map.find(teamname) != map.end())
    {
        std::cout << "[Error]Add failed: duplicated team name.\n";
        return;
    }
    tmpkey.name = tmpteam.name = teamname;
    tmpteam.ac_count = tmpteam.penalty_time = 0;
    for (int i = 0; i < 26; i++) 
        tmpteam.freeze = tmpteam.problems[i].freeze_count = tmpteam.problems[i].status = 0;
    map[teamname] = tmpteam;
    set.insert(tmpkey);
    std::cout<< "[Info]Add successfully.\n";
}

void start()
{
    std::cin >> trash;
    std::cin >> trash;
    std::cin >> trash;
    std::cin >> problem_count;
    flush();
    freeze = false;
    std::cout << "[Info]Competition starts.\n";
}

void flush()
{
    int count = 0;
    for (auto it = set.begin(); it != set.end(); it++)
    {
        count++;
        ranking[count] = it->name;
        map[it->name].rank = count;
    }
}

void update(const std::string& name, const int& number, const int& time)
{
    team *p = &map[name];  
    p->problems[number].status = 1 - p->problems[number].status;
    p->ac_time[p->ac_count] = time;
    p->ac_count++;
    p->penalty_time += time + 20 * (p->problems[number].status - 1); 
}

void submit()
{
    std::cin >> tmpsub.problem_name;
    std::cin >> trash;
    std::cin >> teamname;
    std::cin >> trash;
    std::cin >> tmpsub.status;
    std::cin >> trash;
    std::cin >> tmpsub.time;
    tmpkey.name = teamname;
    team *p = &map[teamname];
    p->history.push_back(tmpsub);
    int number = tmpsub.problem_name - 'A';
    if (p->problems[number].status <= 0)
    {
        if (freeze)
        {
            if (!p->freeze)
            { 
                p->freeze = true;
                freeze_teams.insert(tmpkey);
            }
            p->freeze_problems.insert(number);
            p->problems[number].freeze_count++;
            if (tmpsub.status == "Accepted") p->problems[number].freeze_status.push_back(tmpsub.time);
            else p->problems[number].freeze_status.push_back(0);
        }
        else
        {
            if (tmpsub.status == "Accepted")
            {
                set.erase(tmpkey);
                update(teamname, number, tmpsub.time);
                set.insert(tmpkey);
            }
            else
            {
                p->problems[number].status--;
            }
        }
    }
}

void Freeze()
{
    if (freeze) std::cout << "[Error]Freeze failed: scoreboard has been frozen.\n";
    else
    {
        freeze = true;
        std::cout << "[Info]Freeze scoreboard.\n";
    }
}

void scroll()
{
    if (!freeze)
    {
        std::cout << "[Error]Scroll failed: scoreboard has not been frozen.\n";
        return;
    }
    std::cout << "[Info]Scroll scoreboard.\n";
    flush();
    printlist();
    std::string nextteam;
    for (auto it = freeze_teams.rbegin(); it != freeze_teams.rend(); )
    {
        teamname = it->name;
        nextteam = (++set.find(*it))->name;
        unfreeze(teamname, *map[teamname].freeze_problems.begin(), nextteam);
        it = freeze_teams.rbegin();
    }
    flush();
    printlist();
    freeze = false;
}

void unfreeze(const std::string& name, const int& number, const std::string& nextteam)
{
    tmpkey.name = teamname = name;
    freeze_teams.erase(tmpkey);
    set.erase(tmpkey);
    team *p = &map[teamname];
    for (int i = 0; i < p->problems[number].freeze_count; i++)
    {
        if (p->problems[number].status <= 0)
        {
            if (p->problems[number].freeze_status[i])
            {
                update(teamname, number, p->problems[number].freeze_status[i]);
                std::sort(p->ac_time, p->ac_time + p->ac_count);
            }
            else p->problems[number].status--;
        }
    }
    p->problems[number].freeze_status.clear();
    p->problems[number].freeze_count = 0;
    p->freeze_problems.erase(number);
    if (p->freeze_problems.empty()) p->freeze = false;  
    else freeze_teams.insert(tmpkey);
    auto result = set.insert(tmpkey);
    auto it = result.first;
    if ((++it) != set.end() && it->name != nextteam)
    {
        std::cout << teamname << ' ' << it->name << ' ' << p->ac_count << ' ' << p->penalty_time << '\n';
    }
}

void printlist()
{
    for (int i = 1; ranking[i] != "\0"; i++)
    {
        teamname = ranking[i];
        std::cout << teamname << ' ' << i << ' ' << map[teamname].ac_count << ' ' << map[teamname].penalty_time << ' ';
        for (int i = 0; i < problem_count; i++)
        {
            tmppro = map[teamname].problems[i];
            if (tmppro.freeze_count == 0)
            {
                if (tmppro.status > 0)
                {
                    if (tmppro.status == 1) std::cout << '+';
                    else std::cout << '+' << tmppro.status - 1;
                }
                else
                {
                    if (tmppro.status == 0) std::cout << '.';
                    else std::cout << tmppro.status;
                }
            } 
            else
            {
                std::cout << tmppro.status << '/' << tmppro.freeze_count;
            }
            std::cout << ' ';
        }
        std::cout << '\n';
    }
}

void query_ranking()
{
    std::cin >> teamname;
    if (map.find(teamname) == map.end())
    {
        std::cout << "[Error]Query ranking failed: cannot find the team.\n";
        return;
    }
    std::cout << "[Info]Complete query ranking.\n";
    if (freeze) std::cout << "[Warning]Scoreboard is frozen. The ranking may be inaccurate until it were scrolled.\n";
    std::cout << teamname << " NOW AT RANKING " << map[teamname].rank << '\n';
}

void query_submission()
{
    std::string problem_name, status;
    std::cin >> teamname;
    std::getline(std::cin, trash, '=');
    std::cin >> problem_name;
    std::getline(std::cin, trash, '=');
    std::cin >> status;
    if (map.find(teamname) == map.end())
    {
        std::cout << "[Error]Query submission failed: cannot find the team.\n";
        return;
    }
    std::cout << "[Info]Complete query submission.\n";
    for (auto it = map[teamname].history.rbegin(); it != map[teamname].history.rend(); it++)
    {
        if (problem_name != "ALL")
            if (it->problem_name != problem_name[0])
                continue;
        if (status != "ALL")
            if (it->status != status)
                continue;
        std::cout << teamname << ' ' << it->problem_name << ' ' << it->status << ' ' << it->time << '\n';
        return;
    }
    std::cout << "Cannot find any submission.\n";
}
