#include "pugixml.hpp"

#include <iostream>
#include <vector>

struct Task {
    int mDeadline;
    int mId;
    int mPeriod;
    int mWcet;
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

int main()
{
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file("Exercise1/small.xml");
    if(!result)
        std::cout << "Didn't find the specified file" << std::endl;


    for(pugi::xml_node readinTask : doc.child("Model").child("Application").children("Task"))
    {
        Task t;
        t.mDeadline = readinTask.attribute("Deadline").as_int();
        t.mId = readinTask.attribute("Id").as_int();
        t.mPeriod = readinTask.attribute("Period").as_int();
        t.mWcet = readinTask.attribute("WCET").as_int();

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
}