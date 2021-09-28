#include "pugixml.hpp"

#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

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
   vector<Core> mCores;
}typedef MCP;

vector<Task> tasks;
vector<MCP> platform;

void readIn(string fileName)
{
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(fileName.c_str());
    if(!result)
        cout << "Didn't find the specified file" << endl;

    for(pugi::xml_node readinTask : doc.child("Model").child("Application").children("Task"))
    {
        Task t;
        t.mDeadline = readinTask.attribute("Deadline").as_int();
        t.mId = readinTask.attribute("Id").as_int();
        t.mPeriod = readinTask.attribute("Period").as_int();
        t.mWcet = readinTask.attribute("WCET").as_int();
        t.mPriority = 1.0 / (double)t.mDeadline;

        tasks.push_back(t);
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
    cout << "Read in success" << endl;
}

bool checkIfAllCoreHasTasks(vector<pair<string, string>> solution)
{
    for(unsigned i = 0; i < platform.size(); ++i)
    {
        for(unsigned j = 0; j < platform.at(i).mCores.size(); ++j)
        {
            if(find_if(solution.begin(), solution.end(), [i, j](pair<string, string > pair){
                return pair.second.compare(to_string(i) + "." + to_string(j)) == 0;
                }) == solution.end())
                return false;
        }
    }

    return true;
}

//first task id, second mcpid.coreid
vector<pair<string, string>> createInitialSolution()
{
    vector<pair<string, string>> solution;

    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> mcpRandom(0, platform.size()-1);
    while(!checkIfAllCoreHasTasks(solution))
    {
        solution.clear();
        for(unsigned i = 0; i < tasks.size(); ++i)
        {
            int mcp = mcpRandom(rng);
            uniform_int_distribution<mt19937::result_type> coreRandom(0, platform.at(mcp).mCores.size() - 1);
            int core = coreRandom(rng);

            solution.push_back(make_pair(to_string(i), to_string(mcp) + "." + to_string(core)));
        }
    }

    cout << "Initial solution created" << endl;
   return solution;
}

double calculateWCRT(vector<pair<string, string>> solution) {
    // calculate WCRT for each task
    double wcrt = 0.0;
    // 
    return wcrt;
}


int main()
{
    readIn("Exercise1/small.xml");

    createInitialSolution();

    return 0;
}