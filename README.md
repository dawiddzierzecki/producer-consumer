# producer-consumer
# Description of program operation

The program consists of three processes P1, P2 and P3 and an unnamed link and a message queue.

The P1 process is responsible for reading data from the standard input stream and passing it to the P2 process via the unnamed link. During operation, the P1 process waits for the receipt of SIGUSR1, SIGUSR2, SIGINT signals, and depending on the received signal, it pauses or resumes operation, or terminates its operation.

Process P2 receives data from process P1 via an unnamed link and counts how many words are in the transmitted message. The result of the action is sent by process P2 to process P3 via a
message queue. As with process P1, process P2 waits for SIGUSR1, SIGUSR2, SIGINT signals.

The P3 process receives the result of the P2 process from the message queue and displays it on the standard output.

The code uses signals to communicate between processes and control their operation. The signal handling function is installed using the signal() function. While the program is running, processes can be paused or resumed using SIGUSR1 and SIGUSR2 signals. This makes it possible, for example, to pause data entry and restart it while the program is running. Processes can also be terminated with the SIGINT signal.
The code also uses a message queue to pass information between P2 and P3 processes. The msgget() function was used to create the queue, and msgrcv() and msgsnd() to read and send messages.

Finally, the program is structurally divided into three processes, where each process is responsible for different tasks. The program is controlled by signals that allow the processes to pause, resume and terminate the program.
