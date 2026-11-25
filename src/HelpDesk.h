// HelpDesk
// The help desk itself. Takes a Ticket queue and an action stack

//   Little helpful key :)
//   Command        What it does (TicketQueue)           (Action stack)
//   add            enqueue at the rear                  push ADD
//   serve          dequeue from the front               push SERVE
//   print          walk front -> rear                   -
//   search         walk front -> rear, add up minutes   -
//   cancel         removeById (rotate once around)      push CANCEL
//   undo           remove / insertAt to reverse         pop the last action
//   load / save    rebuild / walk the queue             load clears it
//   simulate       dequeue from a COPY of the queue     -

#ifndef HELPDESK_H
#define HELPDESK_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include "Ticket.h"
#include "TicketQueue.h"
#include "ActionStack.h"

class HelpDesk
{
private:
    TicketQueue waiting;    // tickets in line
    ActionStack history;    // actions that can be undone
    int nextId = 1;         // id given to the next new ticket
    int servedCount = 0;    // tickets served so far today

public:
    const TicketQueue &queue() const {return waiting;}
    const ActionStack &actions() const {return history;}
    int served() const {return servedCount;}

    // -------- ADD: enqueue a new ticket with the next id  -------- 
    void add(const std::string &name, const std::string &category, int minutes){
        // build the ticket
        Ticket t;
        t.id = nextId;
        t.name = name;
        t.category = category;
        t.minutes = minutes;
        
        // QUEUE: add it at the rear
        if (!waiting.enqueue(t))
            return;
        nextId++;

        // STACK: write down what we did
        Action a;
        a.type = ACTION_ADD;
        a.ticket = t;
        history.push(a);
        std::cout << "  added ticket #" << t.id << " for " << t.name << std::endl;
    }

    //  --------  Dequeue the front ticket and record as served  -------- 
    void serve(){
        // QUEUE: remove the front ticket
        Ticket t;
        if (!waiting.dequeue(t)){           // empty queue: nothing to serve, and nothing to undo later
            std::cout << "  no tickets waiting" << std::endl;
            return;
        }
        servedCount++;          //record it as served

        //S TACK: write down what we did, so undo can put this ticket back
        Action a;
        a.type = ACTION_SERVE;
        a.ticket = t;
        history.push(a);
        std::cout << "  serving ticket #" << t.id << ": " << t.name
                  << " (" << t.category << ", " << t.minutes << " min)" << std::endl;
    }

    //  --------  Print: list the waiting tickets from front to rear with the total minutes  -------- 
    void print() const{
        // QUEUE: TicketQueue: print() lists every ticket and the total minutes
        waiting.print();
        std::cout << "  served today: " << servedCount << std::endl;
    }

    //  --------  Search id: position in line and estimated wait  -------- 
    void searchId(int id) const{
        // QUEUE: TicketQueue :searchId walks the line from the front
        int wait;
        int pos = waiting.searchId(id, wait);
        if (pos == -1)      // id was not found
            std::cout << "  ticket #" << id << " is not waiting" << std::endl;
        else                // wait = minutes of the tickets in front of this one
            std::cout << "  ticket #" << id << " is at position " << pos
                      << ", estimated wait " << wait << " minutes" << std::endl;
    }

    //  --------  Search name: position in line and estimated wait  -------- 
    void searchName(const std::string &name) const{
        // QUEUE. Returns the position (or -1) and fills in wait
        int wait;
        int pos = waiting.searchName(name, wait);
        if (pos == -1)    //-1 means no waiting ticket has exactly this name
            std::cout << "  no ticket waiting for " << name << std::endl;
        else            //wait = minutes of the tickets IN FRONT of this one
            std::cout << "  " << name << " is at position " << pos
                      << ", estimated wait " << wait << " minutes" << std::endl;
    }

    //  --------  Cancel id: delete a ticket from anywhere in the line  -------- 
    void cancel(int id){
        //QUEUE: removeById rotates once around the queue and drops this ticket
        Ticket t;
        int pos;
        if (!waiting.removeById(id, t, pos)){           // id not in the line
            std::cout << "  ticket #" << id << " is not waiting" << std::endl;
            return;
        }

        //STACK: write down what we did
        Action a;
        a.type = ACTION_CANCEL;
        a.ticket = t;
        a.position = pos;      // remember where it was, so undo can put it back
        history.push(a);
        std::cout << "  cancelled ticket #" << id << " (was at position " << pos << ")" << std::endl;
    }

    //  --------  Undo: pop the last action and do the opposite  -------- 
    // ADD -> remove that ticket again (it is at the rear) and reuse its id
    // SERVE -> put the ticket back at the front 
    // CANCEL -> put the ticket back at its old position
    void undo(){
        //STACK: pop the most recent action
        Action a;
        if (!history.pop(a)){           // empty stack: nothing has been done yet
            std::cout << "  nothing to undo" << std::endl;
            return;
        }

        if (a.type == ACTION_ADD){      // take the added ticket out again, found by its id
            Ticket t;
            int pos;
            waiting.removeById(a.ticket.id, t, pos);
            nextId--;       // give the id back, so the next add reuses it
            std::cout << "  undo add: removed ticket #" << a.ticket.id << std::endl;
        }
        else if (a.type == ACTION_SERVE){       // QUEUE: the served ticket was at the front, so put it back at position 1
            waiting.insertAt(1, a.ticket);
            servedCount--;      // it is no longer counted as served
            std::cout << "  undo serve: ticket #" << a.ticket.id << " is back at the front" << std::endl;
        }
        else{       //put the ticket back where it was, using the position saved when it was cancelled
            waiting.insertAt(a.position, a.ticket);
            std::cout << "  undo cancel: ticket #" << a.ticket.id
                      << " is back at position " << a.position << std::endl;
        }
    }

    // --------  Load file: start a new day from a CSV file  -------- 
    // can't be undone 
    // File format (first line is a header):
    //   id,name,category,minutes
    //   1,Ana Lopez,Password,5
    void load(const std::string &filename){
        // 1. open the file for reading; stop if it doesn't exist
        std::ifstream in(filename);
        if (!in){
            std::cout << "  cannot open " << filename << std::endl;
            return;
        }

        // 2. new day: empty the queue and the undo history, reset the counters.
        waiting.clear();
        history.clear();
        servedCount = 0;
        nextId = 1;

        // 3. read the file one line at a time
        std::string line;
        std::getline(in, line);    // skip the header line
        int lineNumber = 1;
        while (std::getline(in, line)){
            lineNumber++;
            if (line.empty())
                continue;       // skip blank lines, go to the next line

            // 4. split the line at the commas
            std::stringstream ss(line);
            std::string idText, name, category, minutesText;
            std::getline(ss, idText, ',');
            std::getline(ss, name, ',');
            std::getline(ss, category, ',');
            std::getline(ss, minutesText, ',');

            // 5. turn the number fields from text into ints
            Ticket t;
            try{
                t.id = std::stoi(idText);
                t.minutes = std::stoi(minutesText);
            }
            catch (...){        // bad number: skip this line instead of crashing
                std::cout << "  line " << lineNumber << " skipped (bad number)" << std::endl;
                continue;
            }
            t.name = name;
            t.category = category;

            // 6. QUEUE: add the ticket at the rear
            if (!waiting.enqueue(t))
                break;
            if (t.id >= nextId)
                nextId = t.id + 1;    // new tickets continue after the highest id
        }
        std::cout << "  loaded " << waiting.size() << " ticket(s) from " << filename << std::endl;
    }

    // --------  Save file: write the waiting tickets  in the same CSV format that load reads  -------- 
    void save(const std::string &filename) const{
        // 1. open the file for writing
        std::ofstream out(filename);
        if (!out){
            std::cout << "  cannot write " << filename << std::endl;
            return;
        }

        // 2. write the header line first, so load can skip it
        out << "id,name,category,minutes" << std::endl;
        
        // 3. QUEUE: walk the line from front to rear with get(), which only reads
        Ticket t;
        for (int pos = 1; pos <= waiting.size(); pos++){
            waiting.get(pos, t);         // copy the ticket at this position into t
            // one ticket per line, fields separated by commas
            out << t.id << "," << t.name << "," << t.category << "," << t.minutes << std::endl;
        }
        std::cout << "  saved " << waiting.size() << " ticket(s) to " << filename << std::endl;
    }

    //  --------  Simulate -------- 
    // serve the whole waiting line with a number of agents and report the average and the longest wait.
    void simulate(int agents) const{
        const int MAX_AGENTS = 10;
        if (agents < 1 || agents > MAX_AGENTS){
            std::cout << "  agents must be between 1 and " << MAX_AGENTS << std::endl;
            return;
        }
        if (waiting.isEmpty()){
            std::cout << "  no tickets to simulate" << std::endl;
            return;
        }

        TicketQueue line = waiting;      // copy, so the real queue stays as it is
        int freeAt[MAX_AGENTS] = {0};   // minute at which each agent is free again
        int tickets = 0;
        int totalWait = 0;
        int longestWait = 0;
        int longestId = 0;

        Ticket t;
        while (line.dequeue(t)){
            // find the agent who is free first
            int best = 0;
            for (int a = 1; a < agents; a++)
                if (freeAt[a] < freeAt[best])
                    best = a;

            int wait = freeAt[best];   // the ticket waits until that agent is free
            freeAt[best] += t.minutes; // the agent is busy for t.minutes

            tickets++;
            totalWait += wait;
            if (wait > longestWait){
                longestWait = wait;
                longestId = t.id;
            }
        }

        // day ends when the last agent finishes
        int dayEnds = 0;
        for (int a = 0; a < agents; a++)
            if (freeAt[a] > dayEnds)
                dayEnds = freeAt[a];

        std::cout << std::fixed << std::setprecision(1);
        std::cout << "  simulation with " << agents << " agent(s), " << tickets << " tickets" << std::endl;
        std::cout << "    average wait: " << (double)totalWait / tickets << " minutes" << std::endl;
        std::cout << "    longest wait: " << longestWait << " minutes (ticket #" << longestId << ")" << std::endl;
        std::cout << "    last ticket finished at minute " << dayEnds << std::endl;
        std::cout << std::defaultfloat;
    }
};

#endif
