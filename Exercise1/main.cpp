#include "pugixml.hpp"

#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <algorithm>
#include <string>
#include <tuple>
#include <math.h>

struct Task {
    int mDeadline;
    int mId;
    int mPeriod;
    double mWcet;
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

//read in the html file and save the data
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

//calculate greatest common divisor
long long gcd(long long int a, long long int b)
{
    if(b == 0)
        return a;
    return gcd(b, a % b);
}

//calculate least common multiple
long long lcm(long long a, long long b)
{
    return (a / gcd(a, b)) * b;
}

//See if the tasks on a core meet the deadline
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

//create an initial solution, where in the vector the first is task id, second mcpid third coreid
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

//swap two tasks
std::vector<std::tuple<int, int, int>> selectRandomNeighbourhoodSwap(std::vector<std::tuple<int, int, int>> solution) 
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> generate(0, solution.size() - 1);
    unsigned i = generate(rng);
    unsigned j = generate(rng);

    std::swap(std::get<0>(solution[i]), std::get<0>(solution[j]));
    return solution;
}

//move a task from one core to an other
std::vector<std::tuple<int, int, int>> selectRandomNeighbourhoodMove(std::vector<std::tuple<int, int, int>> solution) 
{
    int counter = 0;
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> generate_mcp(0, platform.size() - 1);
    std::uniform_int_distribution<std::mt19937::result_type> generate_task(0, tasks.size() - 1);

    std::vector<std::tuple<int, int, int>> newSolution = solution;
    do {
        unsigned mcp = generate_mcp(rng);
        std::uniform_int_distribution<std::mt19937::result_type> generate_core(0, platform.at(mcp).mCores.size() - 1);
        unsigned core = generate_core(rng);
        unsigned task = generate_task(rng);



        std::get<1>(newSolution[task]) = mcp;
        std::get<2>(newSolution[task]) = core;
        counter++;

        if (checkIfAllCoreHasTasks(newSolution))
        {
            return newSolution;
        }

    } while (counter < 50);

    return selectRandomNeighbourhoodSwap(solution);
}

std::vector<std::tuple<int, int, int>> selectRandomNeighbourhoodSolution(int random, std::vector<std::tuple<int, int, int>> solution) 
{
    if (random % 2 == 0) 
    {
        return selectRandomNeighbourhoodMove(solution);
    }
    return selectRandomNeighbourhoodSwap(solution);
}

bool calculateProbability(double delta, double temp) 
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<double> generate(0.0, 1.0);

    double exponential = exp((-1 / temp) * delta);
    double probability = generate(rng);
    return exponential >= probability;
}


std::vector<std::tuple<int, int, int>> runSimulatedAnnealing(std::vector<std::tuple<int, int, int>> &initialSolution)
{
    double temp = 30000000;
    double alpha = 0.995;
    int n = 0;
    double delta = 0.0;
    double solutionLaxity = 0.0;
    double randomSolutionLaxity = 0.0;
    std::vector<std::tuple<int, int, int>> solution = initialSolution;
    while (temp > 1) 
    {
        // check deadlines are met
        // if deadlines are not met, do not change temperature, generate new random solution
        n++;
        std::vector<std::tuple<int, int, int>> randomSolution;
        do {
            randomSolution = selectRandomNeighbourhoodSolution(n, solution);
        } while (!checkDeadline(randomSolution));

        //if deadlines are met, run cost function to calculate laxity for both solutions
        solutionLaxity = calculateLaxity(solution);
        randomSolutionLaxity = calculateLaxity(randomSolution);

        // calculate delta
        delta = solutionLaxity - randomSolutionLaxity;

        if (delta < 0 || calculateProbability(delta, temp))
        {
            solution = randomSolution;
        }
        temp *= alpha;
    }

    return solution;
}

//save solution to xml
void writeOutput(std::vector<std::tuple<int, int, int>> aSolution, std::string aFilepath)
{
    std::sort(aSolution.begin(), aSolution.end(), [](std::tuple<int, int, int> sol1, std::tuple<int, int, int> sol2)
    {
            if(std::get<1>(sol1) == std::get<1>(sol2))
            {
                return std::get<2>(sol1) < std::get<2>(sol2);
            }
            else
            {
                return std::get<1>(sol1) < std::get<1>(sol2);
            };
    });

    pugi::xml_document doc;

    pugi::xml_node node = doc.append_child("solution");

    for(auto& sol : aSolution)
    {
        auto taskNode = node.append_child("Task");

        taskNode.append_attribute("Id") = std::get<0>(sol);
        taskNode.append_attribute("MCP") = std::get<1>(sol);
        taskNode.append_attribute("Core") = std::get<2>(sol);
        taskNode.append_attribute("WCRT") = round(tasks.at(std::get<0>(sol)).mWcet*platform.at(std::get<1>(sol)).mCores.at(std::get<2>(sol)).mWcetFactor);
    }
    

    std::string laxity = "Total Laxity: " + std::to_string((int)round(calculateLaxity(aSolution)));

    doc.insert_child_after(pugi::node_comment, node).set_value(laxity.c_str());

    doc.print(std::cout);

    std::string filename = "solution_" + aFilepath;
    
    doc.save_file(filename.c_str(), PUGIXML_TEXT("  "));

    std::cout << filename << std::endl;
}

int main()
{
    //add file path here
    std::string filepath = "large.xml";
    if(!readIn(filepath))
        return -1;

    std::vector<std::tuple<int, int, int>> initialSolution = createInitialSolution();
    auto solution = runSimulatedAnnealing(initialSolution);

    writeOutput(solution, filepath);

    return 0;
}