What I learned Day 5:
-----------------------------------------------------

                 IPC
                  │
      ┌───────────┼──────────────┌──────────────────────────────|
      │           │              │                              |
     Pipe        FIFO       Shared Memory                Message queue
      │           │              │
   related    independent    very fast
  processes    processes     shared data
      |___________|              |
            |                Shared memory
         Process A          ┌─────────────────┐
            │               │ State = RUNNING │
            │ write()       │ Temp  = 75      │
            ▼               │ Pressure = 120  │
           PIPE             └─────────────────┘
            │                   ▲         ▲
            │ read()            │         │
            ▼               Process A  Process B
         Process B