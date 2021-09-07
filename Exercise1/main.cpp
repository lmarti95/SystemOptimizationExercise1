#include "pugixml.hpp"
#include <iostream>

int main()
{
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file("Exercise1/small.xml");
    if(!result)
        return -1;

    for(pugi::xml_node task : doc.child("Model").child("Application").children("Task"))
    {
        int timeout = task.attribute("Deadline").as_int();

        std::cout << timeout << std::endl;
    }
}