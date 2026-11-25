// A stack of actions - this file acts as an undo button

#ifndef ACTIONSTACK_H
#define ACTIONSTACK_H

#include <iostream>
#include "Ticket.h"

// What kind of action was done
enum ActionType{
    ACTION_ADD,
    ACTION_SERVE,
    ACTION_CANCEL
};

class Action   // things to know to undo an action
{
public:
    ActionType type = ACTION_ADD;   // which action to undo
    Ticket ticket;      // the ticket that was added, served or cancelled
    int position = 0;   // for CANCEL. where the ticket was in the line
};

class ActionStack       // a pile (or stack lol) of actions that were done
{
public:
    static const int CAPACITY = 100;   // cap on actions that can be undone

private:                        // in private so functions can only change data from push or pop
    Action data[CAPACITY];    
    int top = -1;               // stack starts empty

public:
    bool isEmpty() const {return top == -1;}        //used by delete / pop
    bool isFull() const {return top == CAPACITY - 1;}       // used by add/push
    int size() const {return top + 1;}          // how mnay actions are in the stack

    void clear() {top = -1;}        // used to load new day

    // ------- ADD: put a on top. move top up, then store ------- 
    bool push(const Action &a){
        if (isFull()){      //no room: stop before writing past the array
            std::cout << "  undo history is full: this action cannot be undone" << std::endl;
            return false;
        }
        top++;
        data[top] = a;
        return true;
    }

    // -------  DELETE: remove the top action and give it back in a ------- 
    bool pop(Action &a){        //nothing there: stop before reading data[-1]
        if (isEmpty())
            return false;               // underflow: nothing to undo
        a = data[top];
        top--;
        return true;
    }

    // -------  Look at the top action without removing it ------- 
    bool peek(Action &a) const{
        if (isEmpty())
            return false;
        a = data[top];
        return true;
    }

    // ------- PRINT - prints every action on the undo stack (top down) ------- 
    // Instructions say to have this, but it doesn't really seem useful in my opinion. 
    // I figure "Ticket, TicketQueue, ActionStack with add, delete, print, search" literally means they each have 
    // add delete search print. Would make no sense for Ticket though so skip that one. 
    void print() const{
        std::cout << "  top -> ";
        for (int i = top; i >= 0; i--){
            const Action &a = data[i];
            if (a.type == ACTION_ADD)
                std::cout << "add#";
            else if (a.type == ACTION_SERVE)
                std::cout << "serve#";
            else
                std::cout << "cancel#";

            std::cout << a.ticket.id << " ";
        }
        std::cout << "<- bottom   (size " << size() << ")" << std::endl;
    }

    // -------  SEARCH: position of the most recent action on this ticket id ------- 
    // same note as print. Search by ticket id because why not. It's a search function
    int search(int ticketId) const{
        for (int i = top; i >= 0; i--)
            if (data[i].ticket.id == ticketId)  //if this action is about the ticket we want, return its position and stop
                return top - i + 1;
        return -1;
    }
};

#endif
