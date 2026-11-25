# helpdesk-queue-cpp

A help desk ticket system in C++, built on my own queue and stack classes
(no `std::queue` or `std::stack`).

## The problem

A university IT help desk receives requests all day: password resets, broken printers, Wi-Fi problems.
Tickets must be served in arrival order, agents must be able to undo their last action, and the manager
wants to search, cancel and print tickets at any time.

## Project layout

```
helpdesk-queue-cpp/
├── src/
│   ├── Ticket.h        one ticket: id, name, category, minutes
│   ├── TicketQueue.h   circular queue of tickets
│   ├── ActionStack.h   array stack of actions (for undo)
│   ├── HelpDesk.h      every command, using the queue and the stack
│   ├── main.cpp        command loop (reads what you type)
│   └── tests.cpp       unit tests
├── data/
│   └── day1.csv        a sample day of 20 tickets
└── README.md
```

## Class design

| Class | What it is | Main operations |
|---|---|---|
| `Ticket` | A plain record: `id`, `name`, `category`, `minutes` | none (data only) |
| `TicketQueue` | **Circular queue**, capacity 100 | `enqueue`, `dequeue`, `peek`, `print`, `searchId`, `searchName`, `removeById`, `insertAt` |
| `ActionStack` | **Stack**, capacity 100 | `push`, `pop`, `peek`, `print`, `search` |
| `HelpDesk` | Holds one `TicketQueue` (the line) and one `ActionStack` (undo history) | one method per command |

### Which operation uses the queue and which uses the stack

| Command | Queue | Stack |
|---|---|---|
| `add` | `enqueue` the new ticket at the rear | push an ADD action |
| `serve` | `dequeue` the ticket at the front | push a SERVE action |
| `print` | walk front → rear, add up minutes | – |
| `search id` / `search name` | walk front → rear; the wait is the sum of minutes in front of the ticket | – |
| `cancel` | `removeById`: rotate once around the queue, dropping that ticket | push a CANCEL action |
| `undo` | reverse the action | **pop** the last action |
| `load` | clear, then `enqueue` each line of the file | cleared (a load can't be undone) |
| `save` | walk front → rear and write each ticket | – |
| `simulate` | `dequeue` from a copy of the queue | – |

### How the circular queue works

`front` is the index of the first ticket, `rear` the index of the last one.
Both move forward with `% CAPACITY`, so after the last cell of the array they
wrap around to cell 0 and no space is wasted. A `count` tells an empty queue
(`count == 0`) from a full one (`count == CAPACITY`). This is the
`CircularQueue` from lecture section 13.1, storing `Ticket` objects instead of `int`s.

### How cancel removes from the middle (F5)

A queue can only remove at the front, so `removeById` uses the rotation
method from lecture section 12: dequeue every ticket once and enqueue it
again, **except** the one being cancelled. After `count` turns the queue has
gone all the way round, so the other tickets are back in their original order.

```
cancel 3 on   1 2 3 4 5
turn 1: take 1, put back  ->  2 3 4 5 1
turn 2: take 2, put back  ->  3 4 5 1 2
turn 3: take 3, DROP it   ->  4 5 1 2
turn 4: take 4, put back  ->  5 1 2 4
turn 5: take 5, put back  ->  1 2 4 5     (order unchanged)
```

### How undo works (F6)

Each action on the stack stores the ticket and, for a cancel, its old position.
Undo pops the top action and does the opposite:

| Action popped | What undo does |
|---|---|
| ADD | remove that ticket (it is at the rear) and give its id back |
| SERVE | put the ticket back at position 1 (the front) |
| CANCEL | put the ticket back at its old position with `insertAt` |

`insertAt` uses the same rotation as cancel: while rotating, the ticket is
enqueued just before the ticket that should come after it.

### Simulation (M4)

`simulate <agents>` serves the whole waiting line with 1 or more agents.
Each time, the ticket at the front goes to the agent who becomes free first.
A ticket's wait is the minute its agent starts on it.

**Assumption:** every ticket is already waiting when the desk opens
(minute 0). This is the same assumption the "estimated wait" in `search` uses.
The simulation works on a copy, so the real queue is not changed.

## Build and run

Needs a C++17 compiler (g++ shown; the same flags as the lecture notes).

```bash
# build the program
g++ -std=c++17 -Wall -Wextra -pedantic -o helpdesk src/main.cpp

# build and run the unit tests
g++ -std=c++17 -Wall -Wextra -pedantic -o tests src/tests.cpp
./tests

# run the program
./helpdesk
```

## Commands

| Command | What it does |
|---|---|
| `add` | add a ticket (asks for name, category, minutes) |
| `serve` | serve the ticket at the front |
| `print` | list the waiting tickets and total minutes |
| `search id <id>` | position and estimated wait of a ticket |
| `search name <name>` | same, by student name |
| `cancel <id>` | remove a ticket from the line |
| `undo` | undo the last add, serve or cancel |
| `load <file>` | start the day from a CSV file (clears the queue and undo history) |
| `save <file>` | save the waiting tickets to a CSV file |
| `simulate <agents>` | average and longest wait for the current line |
| `help` | show the commands |
| `quit` | exit |

CSV format (used by both `load` and `save`):

```
id,name,category,minutes
1,Ana Lopez,Password,5
2,Ben Carter,Wi-Fi,15
```

## Sample session

(Shortened: `...` marks rows left out of the long `print` listings.)

```
Help Desk Ticket System (type 'help' for commands)
> load data/day1.csv
  loaded 20 ticket(s) from data/day1.csv
> serve
  serving ticket #1: Ana Lopez (Password, 5 min)
> serve
  serving ticket #2: Ben Carter (Wi-Fi, 15 min)
> add
  name: Uma Rivera
  category: Wi-Fi
  minutes: 10
  added ticket #21 for Uma Rivera
> search id 7
  ticket #7 is at position 5, estimated wait 47 minutes
> search name Maya Singh
  Maya Singh is at position 11, estimated wait 118 minutes
> cancel 7
  cancelled ticket #7 (was at position 5)
> print
  pos  id   name              category    minutes
  1    3    Chloe Nguyen      Printer     20
  2    4    David Kim         Password    5
  3    5    Emma Rossi        Email       10
  4    6    Farid Haddad      Wi-Fi       12
  5    8    Hugo Martin       Password    4
  ...
  18   21   Uma Rivera        Wi-Fi       10
  18 ticket(s) waiting, total 206 minutes
  served today: 2
> undo
  undo cancel: ticket #7 is back at position 5
> undo
  undo add: removed ticket #21
> undo
  undo serve: ticket #2 is back at the front
> print
  pos  id   name              category    minutes
  1    2    Ben Carter        Wi-Fi       15
  2    3    Chloe Nguyen      Printer     20
  ...
  6    7    Grace Okafor      Software    25
  ...
  19   20   Tara Walsh        Password    5
  19 ticket(s) waiting, total 236 minutes
  served today: 1
> save data/closing.csv
  saved 19 ticket(s) to data/closing.csv
> quit
```

## Testing (milestones M1–M3)

`./tests` checks:

- **M1:** `TicketQueue` and `ActionStack` when empty (underflow), with one element, and when full (overflow), plus wrap-around in the circular array.
- **M2:** cancel at the front, middle and rear, and the other tickets keep their order.
- **M3:** undo of each command, and **three undos in a row restore the earlier queue exactly**. Also checks that `save` then `load` gives back the same queue.

All 35 checks pass.

## Waiting-time results (M4)

Simulation of `data/day1.csv` (20 tickets, 241 minutes of work in total),
all tickets waiting at opening:

| Agents | Average wait | Longest wait | Last ticket finished |
|---|---|---|---|
| 1 | 115.5 min | 236 min (ticket #20) | minute 241 |
| 2 | 53.5 min | 112 min (ticket #20) | minute 124 |

To reproduce:

```
> load data/day1.csv
> simulate 1
> simulate 2
```

A second agent cuts the average wait by more than half (115.5 → 53.5 minutes)
and the day finishes in about half the time.

## Complexity

| Operation | Cost |
|---|---|
| add, serve (one enqueue/dequeue + one push) | O(1) |
| print, search, cancel | O(n) |
| undo (one pop + a rotation of the queue) | O(n) |
| simulate | O(n × agents) |
