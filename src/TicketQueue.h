// A queue of tickets built on a circular array

#ifndef TICKETQUEUE_H   // if ticketqueue isn't defined yet...
#define TICKETQUEUE_H   // define it

#include <iostream>
#include <iomanip>
#include <string>
#include "Ticket.h"     // gotta include our ticket class (those are our objects)


class TicketQueue
{
public:
    static const int CAPACITY = 100;   // capacity = most tickets that can wait at once

private:
    Ticket data[CAPACITY];
    int front = 0;    // index of first ticket
    int rear = -1;    // index of last ticket
    int count = 0;    // # tickets waiting

public:
    bool isEmpty() const {return count == 0;}
    bool isFull() const {return count == CAPACITY;}
    int size() const {return count;}

    // ----------- Remove every ticket (used when a new day is loaded) ----------- 
    void clear(){
        front = 0;
        rear = -1;
        count = 0;
    }

    // ----------- ADD: put ticket at rear ----------- 
    bool enqueue(const Ticket &t){
        if (isFull()){
            std::cout << "  queue is full (" << CAPACITY << " tickets)" << std::endl;
            return false;
        }
        rear = (rear + 1) % CAPACITY;   // wrap around to 0 after the last cell - circular queue, boom!
        data[rear] = t;
        count++;
        return true;
    }

    // ----------- DELETE: remove the ticket at the front and give it back in t ----------- 
    bool dequeue(Ticket &t){
        if (isEmpty())
            return false;   // underflow: nothing to remove
        t = data[front];
        front = (front + 1) % CAPACITY;  // next ticket is now the front 
        count--;
        return true;
    }

    // -----------  Look (aka peek) at front ticket ----------- 
    bool peek(Ticket &t) const{
        if (isEmpty())
            return false;
        t = data[front];
        return true;
    }

    // ----------- Who is at each position in line ----------- 
    // Copy the ticket at a position into t.
    // Used by save() in HelpDesk to walk the queue without changing it
    bool get(int position, Ticket &t) const{
        if (position < 1 || position > count)
            return false;
        t = data[(front + position - 1) % CAPACITY];
        return true;
    }

    // -----------  Sum to get wait time for ticket ----------- 
    int totalMinutes() const{
        int total = 0;
        for (int k = 0; k < count; k++)
            total += data[(front + k) % CAPACITY].minutes;
        return total;
    }

    // ----------- PRINT: from the front to the rear ----------- 
    // used claude to help set up this table format
    void print() const{
        if (isEmpty()){     //if no tickets
            std::cout << "  (no tickets waiting)" << std::endl;
            return;
        }
        std::cout << "  " << std::left      //set up printed columns
                  << std::setw(5) << "pos" << std::setw(5) << "id"
                  << std::setw(18) << "name" << std::setw(12) << "category"
                  << "minutes" << std::endl;
        for (int k = 0; k < count; k++){        // print the tickets into those columns
            const Ticket &t = data[(front + k) % CAPACITY];
            std::cout << "  " << std::left
                      << std::setw(5) << k + 1 << std::setw(5) << t.id
                      << std::setw(18) << t.name << std::setw(12) << t.category
                      << t.minutes << std::endl;
        }
        std::cout << "  " << count << " ticket(s) waiting, total "      // total tickets and wait time
                  << totalMinutes() << " minutes" << std::endl;
    }

    // -----------  SEARCH by id: returns the position from the front ----------- 
    // -----------  wait is set to the sum of the minutes of the tickets in front of it ----------- 
    int searchId(int id, int &wait) const{
        wait = 0;
        for (int k = 0; k < count; k++){
            const Ticket &t = data[(front + k) % CAPACITY];
            if (t.id == id)
                return k + 1;     
            wait += t.minutes;      // minutes to wait
        }
        return -1;      // if ticket with that id isnt there
    }

    // -----------  SEARCH by name: same as searchId, but finds the first ticket with the name ----------- 
    int searchName(const std::string &name, int &wait) const{
        wait = 0;
        for (int k = 0; k < count; k++){
            const Ticket &t = data[(front + k) % CAPACITY];
            if (t.name == name)
                return k + 1;
            wait += t.minutes;
        }
        return -1;
    }

    // -----------  DELETE A GIVEN VALUE -----------  
    // Take every ticket out once and put it at the rear, except the ticket with this id
    bool removeById(int id, Ticket &removed, int &position){
        int n = count;
        bool found = false;
        for (int k = 0; k < n; k++){
            Ticket t;
            dequeue(t);         // take out ticket
            if (t.id == id && !found){
                found = true;       // drop it: do not put it back since this is the one we wanna delete
                removed = t;
                position = k + 1;
            }
            else{
                enqueue(t);     // put ticket back in but at the end
            }
        }
        return found;
    }

    // ----------- INSERT AT A POSITION ----------- 
    // take every ticket out and put it back, and when we reach the wanted position, put t in 
    bool insertAt(int position, const Ticket &t){
        if (isFull())
            return false;   // cant insert if queue is at capactiy
        int n = count;
        if (position < 1)       // if position too small, just put it at the front
            position = 1;
        if (position > n + 1)       // if position too big, just put it at the end
            position = n + 1;
        for (int k = 0; k < n; k++){        // rotate every ticket once
            if (k == position - 1)      //insert at position k if k=position
                enqueue(t);      // t goes in just before the k-th ticket
            Ticket y;
            dequeue(y);     //take front ticket out
            enqueue(y);     // put that same ticket at the rear
        }
        if (position == n + 1)
            enqueue(t);        // t belongs at the rear
        return true;
    }
};

#endif
