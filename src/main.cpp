// main.cpp
// this file serves as the command guide - it allows the user to input, delete, search, serve, etc data/tickets
// used Claude to help make this work with my other code files
#include <iostream>
#include <sstream>
#include <string>
#include "HelpDesk.h"
using namespace std;

void printHelp(){
    cout << "Commands:" << endl
         << "  add                  add a ticket (asks for name, category, minutes)" << endl
         << "  serve                serve the ticket at the front" << endl
         << "  print                list the waiting tickets" << endl
         << "  search id <id>       position and estimated wait of a ticket" << endl
         << "  search name <name>   same, by student name" << endl
         << "  cancel <id>          remove a ticket from the line" << endl
         << "  undo                 undo the last add, serve or cancel" << endl
         << "  load <file>          start the day from a CSV file" << endl
         << "  save <file>          save the waiting tickets to a CSV file" << endl
         << "  simulate <agents>    average and longest wait for the current line" << endl
         << "  help                 show this list" << endl
         << "  quit                 exit" << endl;
}

// Remove spaces at the start and end of a string (for names typed by the user).
string trim(const string &s){
    size_t first = s.find_first_not_of(" \t\r");
    if (first == string::npos)
        return "";
    size_t last = s.find_last_not_of(" \t\r");
    return s.substr(first, last - first + 1);
}

int main(){
    HelpDesk desk;
    string line;

    cout << "Help Desk Ticket System (type 'help' for commands)" << endl;
    while (true){
        cout << "> ";
        if (!getline(cin, line))
            break;       // end of input

        stringstream ss(line);
        string command;
        ss >> command;

        if (command.empty())
            continue;
        else if (command == "quit")
            break;
        else if (command == "help")
            printHelp();
        else if (command == "add"){
            string name, category, minutesText;
            cout << "  name: ";
            getline(cin, name);
            cout << "  category: ";
            getline(cin, category);
            cout << "  minutes: ";
            getline(cin, minutesText);

            name = trim(name);
            category = trim(category);
            stringstream ms(minutesText);
            int minutes;
            if (name.empty() || category.empty() || !(ms >> minutes) || minutes <= 0){
                cout << "  invalid ticket: name and category are required, minutes must be > 0" << endl;
                continue;
            }
            desk.add(name, category, minutes);
        }
        else if (command == "serve")
            desk.serve();
        else if (command == "print")
            desk.print();
        else if (command == "search"){
            string kind;
            ss >> kind;
            if (kind == "id"){
                int id;
                if (ss >> id)
                    desk.searchId(id);
                else
                    cout << "  usage: search id <id>" << endl;
            }
            else if (kind == "name"){
                string name;
                getline(ss, name);              // the rest of the line, spaces allowed
                name = trim(name);
                if (name.empty())
                    cout << "  usage: search name <name>" << endl;
                else
                    desk.searchName(name);
            }
            else
                cout << "  usage: search id <id>  or  search name <name>" << endl;
        }
        else if (command == "cancel"){
            int id;
            if (ss >> id)
                desk.cancel(id);
            else
                cout << "  usage: cancel <id>" << endl;
        }
        else if (command == "undo")
            desk.undo();
        else if (command == "load" || command == "save"){
            string filename;
            ss >> filename;
            if (filename.empty())
                cout << "  usage: " << command << " <file>" << endl;
            else if (command == "load")
                desk.load(filename);
            else
                desk.save(filename);
        }
        else if (command == "simulate"){
            int agents;
            if (ss >> agents)
                desk.simulate(agents);
            else
                cout << "  usage: simulate <agents>" << endl;
        }
        else
            cout << "  unknown command, type 'help'" << endl;
    }
    return 0;
}
