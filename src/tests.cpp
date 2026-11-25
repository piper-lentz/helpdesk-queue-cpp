// Unit tests for the milestone checkpoints:
//   M1: TicketQueue and ActionStack work when empty, with one element and when full
//   M2: cancel works at the front, middle and rear; the others keep their order
//   M3: undo works for every command; three undos in a row restore the queue exactly
//
// Each check prints PASS or FAIL. The program returns 0 only if every check passed.

// I used the help of Claude while writing this file. It runs unit tests without needing to type 
// everything in the command line. I reviewed the code to verify it was doing what I expected, and 
// tested it and compared the outputs with my expectations.

#include <iostream>
#include <sstream>
#include <string>
#include "HelpDesk.h"
using namespace std;

int failures = 0;

void check(bool condition, const string &description)
{
    cout << (condition ? "PASS  " : "FAIL  ") << description << endl;
    if (!condition)
        failures++;
}

// Make a ticket quickly.
Ticket makeTicket(int id, int minutes)
{
    Ticket t;
    t.id = id;
    t.name = "Student" + to_string(id);
    t.category = "Test";
    t.minutes = minutes;
    return t;
}

// The ids in the queue from front to rear, e.g. "1 2 3". Easy to compare.
string ids(const TicketQueue &q)
{
    string s;
    Ticket t;
    for (int pos = 1; pos <= q.size(); pos++)
    {
        q.get(pos, t);
        if (!s.empty())
            s += " ";
        s += to_string(t.id);
    }
    return s;
}

// The HelpDesk methods print messages. While testing we hide them by sending
// cout to a string buffer, then put cout back.
stringstream hidden;
streambuf *realCout = nullptr;
void quiet() { realCout = cout.rdbuf(hidden.rdbuf()); }
void loud() { cout.rdbuf(realCout); }

void testQueue()
{
    cout << "--- M1: TicketQueue ---" << endl;
    TicketQueue q;
    Ticket t;
    int wait;

    // empty
    check(q.isEmpty() && q.size() == 0, "new queue is empty");
    check(!q.dequeue(t), "dequeue on empty queue fails (underflow)");
    check(q.searchId(1, wait) == -1, "search on empty queue returns -1");

    // one element
    q.enqueue(makeTicket(1, 10));
    check(q.size() == 1 && q.totalMinutes() == 10, "one ticket: size 1, total 10 minutes");
    check(q.searchId(1, wait) == 1 && wait == 0, "one ticket: position 1, wait 0");
    check(q.dequeue(t) && t.id == 1 && q.isEmpty(), "one ticket: dequeue gives it back, queue empty");

    // full
    for (int i = 1; i <= TicketQueue::CAPACITY; i++)
        q.enqueue(makeTicket(i, 1));
    check(q.isFull() && q.size() == TicketQueue::CAPACITY, "full queue: isFull is true");
    quiet();
    bool added = q.enqueue(makeTicket(999, 1));
    loud();
    check(!added, "enqueue on full queue fails (overflow)");
    check(q.searchId(TicketQueue::CAPACITY, wait) == TicketQueue::CAPACITY &&
              wait == TicketQueue::CAPACITY - 1,
          "full queue: last ticket found at the rear with the right wait");

    // wrap around: take two out, put two in, order must stay FIFO
    q.dequeue(t);
    q.dequeue(t);
    q.enqueue(makeTicket(101, 1));
    q.enqueue(makeTicket(102, 1));
    q.dequeue(t);
    check(t.id == 3, "after wrap-around, tickets still come out in arrival order");

    // search by name and wait
    TicketQueue q2;
    q2.enqueue(makeTicket(1, 5));
    q2.enqueue(makeTicket(2, 10));
    q2.enqueue(makeTicket(3, 15));
    check(q2.searchName("Student3", wait) == 3 && wait == 15, "search name: position 3, wait 5 + 10 = 15");
}

void testStack()
{
    cout << "--- M1: ActionStack ---" << endl;
    ActionStack s;
    Action a;

    // empty
    check(s.isEmpty() && s.size() == 0, "new stack is empty");
    check(!s.pop(a), "pop on empty stack fails (underflow)");
    check(s.search(1) == -1, "search on empty stack returns -1");

    // one element
    a.type = ACTION_SERVE;
    a.ticket = makeTicket(7, 3);
    s.push(a);
    check(s.size() == 1 && s.search(7) == 1, "one action: size 1, found at position 1");
    Action b;
    check(s.pop(b) && b.type == ACTION_SERVE && b.ticket.id == 7 && s.isEmpty(),
          "one action: pop gives it back, stack empty");

    // full
    for (int i = 1; i <= ActionStack::CAPACITY; i++)
    {
        a.ticket = makeTicket(i, 1);
        s.push(a);
    }
    check(s.isFull(), "full stack: isFull is true");
    quiet();
    bool pushed = s.push(a);
    loud();
    check(!pushed, "push on full stack fails (overflow)");
    s.pop(b);
    check(b.ticket.id == ActionStack::CAPACITY, "full stack: pop gives the LAST action pushed (LIFO)");
    check(s.search(1) == ActionStack::CAPACITY - 1, "search: first action pushed is at the bottom");
}

// A desk with tickets 1..5 waiting.
void fillDesk(HelpDesk &desk)
{
    for (int i = 1; i <= 5; i++)
        desk.add("Student" + to_string(i), "Test", i * 5);
}

void testCancel()
{
    cout << "--- M2: cancel ---" << endl;
    quiet();
    HelpDesk front, middle, rear, missing;
    fillDesk(front);
    fillDesk(middle);
    fillDesk(rear);
    fillDesk(missing);
    front.cancel(1);
    middle.cancel(3);
    rear.cancel(5);
    missing.cancel(42);
    loud();

    check(ids(front.queue()) == "2 3 4 5", "cancel at the front: 2 3 4 5");
    check(ids(middle.queue()) == "1 2 4 5", "cancel in the middle: 1 2 4 5");
    check(ids(rear.queue()) == "1 2 3 4", "cancel at the rear: 1 2 3 4");
    check(ids(missing.queue()) == "1 2 3 4 5", "cancel an id that is not waiting: nothing changes");
}

void testUndo()
{
    cout << "--- M3: undo ---" << endl;
    HelpDesk desk;
    quiet();
    fillDesk(desk);
    loud();
    string before = ids(desk.queue());          // "1 2 3 4 5"

    // undo each command on its own
    quiet();
    desk.add("New", "Test", 5);                 // ticket 6
    desk.undo();
    loud();
    check(ids(desk.queue()) == before, "undo add removes the new ticket");
    quiet();
    desk.add("New", "Test", 5);
    loud();
    check(ids(desk.queue()) == "1 2 3 4 5 6", "after undo add, the next ticket reuses id 6");
    quiet();
    desk.undo();
    desk.serve();
    desk.undo();
    loud();
    check(ids(desk.queue()) == before && desk.served() == 0, "undo serve puts the ticket back at the front");
    quiet();
    desk.cancel(3);
    desk.undo();
    loud();
    check(ids(desk.queue()) == before, "undo cancel puts the ticket back at its old position");

    // the M3 checkpoint: three different actions, then three undos
    quiet();
    desk.serve();               // 2 3 4 5
    desk.cancel(4);             // 2 3 5
    desk.add("New", "Test", 5); // 2 3 5 6
    loud();
    check(ids(desk.queue()) == "2 3 5 6", "after serve, cancel 4, add: 2 3 5 6");
    quiet();
    desk.undo();
    desk.undo();
    desk.undo();
    loud();
    check(ids(desk.queue()) == before, "three undos in a row restore 1 2 3 4 5 exactly");
    check(desk.actions().size() == 5, "undo history is back to the 5 original adds");

    HelpDesk fresh;
    quiet();
    fresh.undo();               // nothing to undo: must not crash or change anything
    loud();
    check(fresh.queue().isEmpty() && fresh.actions().isEmpty(), "undo with an empty history changes nothing");
}

void testFiles()
{
    cout << "--- M3: save and load ---" << endl;
    HelpDesk desk, other;
    quiet();
    fillDesk(desk);
    desk.cancel(2);
    desk.save("test_save.csv");
    other.load("test_save.csv");
    loud();
    check(ids(other.queue()) == "1 3 4 5", "load reads back exactly what save wrote");
    check(other.queue().totalMinutes() == desk.queue().totalMinutes(), "minutes are saved and loaded too");
    quiet();
    other.add("New", "Test", 5);
    loud();
    check(ids(other.queue()) == "1 3 4 5 6", "after load, new ids continue after the highest id");
    remove("test_save.csv");
}

int main()
{
    testQueue();
    testStack();
    testCancel();
    testUndo();
    testFiles();
    cout << endl << (failures == 0 ? "All tests passed." : to_string(failures) + " test(s) FAILED.") << endl;
    return failures == 0 ? 0 : 1;
}
