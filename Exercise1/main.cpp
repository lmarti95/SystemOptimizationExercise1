#include "pugixml.hpp"

#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>
#include <tuple>

struct Task {
    int mDeadline;
    int mId;
    int mPeriod;
    int mWcet;
    double mPriority;
}typedef Task;

struct Core {
    int mId;
    double mWcetFactor;
}typedef Core;

struct MCP {
    int mId;
    std::vector<Core> mCores;
}typedef MCP;

std::vector<Task> tasks;
std::vector<MCP> platform;

int deadlineSum = 0;

bool readIn(std::string fileName)
{
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(fileName.c_str());
    if(!result)
    {
        std::cout << "Didn't find the specified file" << std::endl;
        return false;
    }

    for(pugi::xml_node readinTask : doc.child("Model").child("Application").children("Task"))
    {
        Task t;
        t.mDeadline = readinTask.attribute("Deadline").as_int();
        t.mId = readinTask.attribute("Id").as_int();
        t.mPeriod = readinTask.attribute("Period").as_int();
        t.mWcet = readinTask.attribute("WCET").as_int();
        t.mPriority = 1.0 / (double)t.mDeadline;

        tasks.push_back(t);

        deadlineSum += t.mDeadline;
    }

    for(pugi::xml_node readinMCP : doc.child("Model").child("Platform").children("MCP"))
    {
        MCP mcp;
        mcp.mId = readinMCP.attribute("Id").as_int();
        for(pugi::xml_node readinCore : readinMCP.children())
        {
            Core c;
            c.mId = readinCore.attribute("Id").as_int();
            c.mWcetFactor = readinCore.attribute("WCETFactor").as_double();
            mcp.mCores.push_back(c);
        }
        platform.push_back(mcp);
    }
    std::cout << "Read in success" << std::endl;
    return true;
}

bool checkIfAllCoreHasTasks(std::vector<std::tuple<int, int, int>> solution)
{
    for(unsigned i = 0; i < platform.size(); ++i)
    {
        for(unsigned j = 0; j < platform.at(i).mCores.size(); ++j)
        {
            if(std::find_if(solution.begin(), solution.end(), [i, j](std::tuple<int, int, int> element){
                return std::get<1>(element) == i && std::get<2>(element) == j;
                }) == solution.end())
                return false;
        }
    }

    return true;
}

long long gcd(long long int a, long long int b)
{
    if(b == 0)
        return a;
    return gcd(b, a % b);
}

long long lcm(long long a, long long b)
{
    return (a / gcd(a, b)) * b;
}

bool checkCoreDeadline(int i, int j, std::vector<std::tuple<int, int, int>> aSolution)
{
    std::vector<int> taskIDs;
    for(auto& element : aSolution)
    {
        if(std::get<1>(element) == i && std::get<2>(element))
        {
            taskIDs.push_back(std::get<0>(element));
        }
    }

    long long commonDeadline = tasks.at(taskIDs.at(0)).mDeadline;
    if(taskIDs.size() > 1)
    {
        for(unsigned int i = 1; i < taskIDs.size(); ++i)
        {
            commonDeadline = lcm(commonDeadline, tasks.at(taskIDs.at(i)).mDeadline);
        }
    }

    long long resourceSum = 0;

    for(unsigned int i = 0; i < taskIDs.size(); ++i)
    {
        resourceSum += tasks.at(taskIDs.at(i)).mDeadline / commonDeadline * tasks.at(taskIDs.at(i)).mWcet;
    }

    if(resourceSum > commonDeadline)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool checkDeadline(std::vector<std::tuple<int, int, int>> aSolution)
{
    for(unsigned i = 0; i < platform.size(); ++i)
    {
        for(unsigned j = 0; j < platform.at(i).mCores.size(); ++j)
        {
            if(!checkCoreDeadline(i, j, aSolution))
            {
                return false;
            }
        }
    }

    return true;
}

bool check(std::vector<std::tuple<int, int, int>> aSolution)
{
    if(!checkIfAllCoreHasTasks(aSolution))
        return false;
    if(!checkDeadline(aSolution))
        return false;
    return true;
}

//first task id, second mcpid.coreid
std::vector<std::tuple<int, int, int>> createInitialSolution()
{
    std::vector<std::tuple<int, int, int>> solution;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> mcpRandom(0, platform.size()-1);
    while(!check(solution))
    {
        solution.clear();
        for(unsigned i = 0; i < tasks.size(); ++i)
        {
            int mcp = mcpRandom(rng);
            std::uniform_int_distribution<std::mt19937::result_type> coreRandom(0, platform.at(mcp).mCores.size() - 1);
            int core = coreRandom(rng);

            solution.push_back(std::make_tuple(i, mcp, core));
        }
    }

    std::cout << "Initial solution created" << std::endl;
    return solution;
}

double calculateLaxity(std::vector<std::tuple<int, int, int>> aSolution)
{
    double sum = 0;
    for(auto& element : aSolution)
    {
        sum += platform.at(std::get<1>(element)).mCores.at(std::get<2>(element)).mWcetFactor * tasks.at(std::get<0>(element)).mWcet;
    }

    return deadlineSum - sum;
}

int main()
{
    if(!readIn("Exercise1/small.xml"))
        return -1;

    createInitialSolution();

    return 0;
}