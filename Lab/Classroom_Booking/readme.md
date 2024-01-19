# Classroom Booking
Bookings will be done for a day for 5 classrooms.
Timeslots are of 1.5 hours starting from 08:00 - 20:00. They do not overlap.
Cancel request will fail if made within 20 seconds from the time of booking.
In the test case, every second one request is made. The Request Time column is the relative time in seconds from start of program.

PLEASE NOTE: INPUT CASES SHOULD CONTAIN THE TIME IN 24 HOUR FORMAT. For e.g 08:00 or 09:30
```
Status Codes
0  - OK
-1- Slot Already booked
-2- Cancel request received within the cool down period.
-3- Invalid Request
```

# Compiling & Executing
Used lang C.
## Server code:
```
$ gcc -o server server_classroom_booking.c -pthread
$ ./server
```

 ## Client code:
```
$  gcc -o client client_classroom_booking.c
$  ./client <file name.csv>
```

Note, server runs indefinitely.
Server writes in the 'output.csv' file.
