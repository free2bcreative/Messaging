#Messaging service in C++
This is my Messaging program.  It's a server-client program that allows a client to send a message to another person.  The client can also read messages for any given name.
Full information about this lab is provided on the class website:

http://cs360.byu.edu/fall-2013/labs/messaging-service

## Threaded messaging service
This is Project/Lab 2, where we added threads and semaphores to our Lab 1 code.  The program can handle a lot of clients at one time using 10 threads and several semaphores.

### For the TAs:

The two main data structures that I used in this project would be the queue for the clients and a map for the messages:

> Queue: Located in C function in "Server.cc" called "threadTask".  The queue is on line 90.  The places where it is protected by semaphores are from lines 94-105.  

> Map:  My semaphore to protect the Map of messages is contained in "AllUsers.cc" (see line 15, where I initialize the semaphore).  However, the places where I call sem_wait and sem_post are under the "put", "list", "get", and "reset" functions within the "Handler.cc" file.  

>> Put - lines 248 - 254
>> List - lines 265 - 271
>> Get - lines 282 - 299
>> Reset - lines 318 - 321

I hope all of that information made sense.  If not, feel free to email me and ask any questions.  Thanks!