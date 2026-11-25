// A ticket is just a small record of data. TicketQueue stores tickets and the HelpDesk works with them

#ifndef TICKET_H           // if TICKET_H is NOT defined yet...
#define TICKET_H           // define it
#include <string>

class Ticket
{
public:
    int id = 0;            // id (given in arrival order (1, 2, 3, ...))
    std::string name;      // student's name
    std::string category;  // kind of problem
    int minutes = 0;       // minutes needed to solve it
};

#endif
