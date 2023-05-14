/*
Zadanie projektowe nr 33 z przedmiotu Systemy Operacyjne
Autor: Dzierzęcki Dawid
Prowadzący: mgr. inż. Maciej Walczak
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#define MAX_SIZE 1024
// Struktura używana w kolejce komunikatów
typedef struct msgbuf {
long mtype;
char mtext[MAX_SIZE];
int words;
} MsgBuf;
//Zmienne globalne
int isEnd = 0;
int isSuspended = 0;
pid_t pid1, pid2, pid3;
// Funkcja obsługi sygnału od użytkownika
void signalFromUserHandler(int signal) {
switch (signal) {
case SIGUSR1:
isSuspended = 1;
printf("\nOdebrano sygnał SIGUSR1. Wstrzymuję pracę.\n");
/* kill(pid1, SIGSTOP);
kill(pid2, SIGSTOP);
kill(pid3, SIGSTOP);*/
break;
case SIGUSR2:
if (isSuspended == 1) {
printf("\nOdebrano sygnał SIGUSR2. Wznawiam pracę.\n");
/*kill(pid1, SIGCONT);
kill(pid2, SIGCONT);
kill(pid3, SIGCONT);*/
isSuspended = 0;
}
break;
case SIGINT:
isEnd = 1;
printf("\nOdebrano sygnał SIGINT. Kończę pracę.\n");
/*kill(pid1, SIGTERM);
kill(pid2, SIGTERM);
kill(pid3, SIGTERM);*/
break;
default:
printf("\nOdebrano sygnał %d. Nie obsluguje takiego.\n", signal);
break;
}
}
void clearInputBuffer() {
int c;
while ((c = getchar()) != '\n' && c != EOF) {}
}
int main() {
int fd[2], status;
key_t key;
int msgid;
MsgBuf msg;
// Utworzenie łącza nienazwanego
if (pipe(fd) == -1) {
perror("pipe");
exit(EXIT_FAILURE);
}
// Utworzenie klucza dla kolejki komunikatów
if ((key = ftok(".", 'A')) == -1) {
perror("ftok");
exit(EXIT_FAILURE);
}
// Utworzenie kolejki komunikatów
if ((msgid = msgget(key, IPC_CREAT | 0666)) == -1) {
perror("msgget");
exit(EXIT_FAILURE);
}
// Instalacja funkcji obsługi sygnałów
signal(SIGINT, signalFromUserHandler);
signal(SIGUSR1, signalFromUserHandler);
signal(SIGUSR2, signalFromUserHandler);
// Utworzenie procesu 1
if ((pid1 = fork()) == -1) {
perror("fork");
exit(EXIT_FAILURE);
}
else if (pid1 == 0) {
// Kod procesu 1
char buffer[MAX_SIZE];
close(fd[0]); // Zamknięcie strony odczytu łącza
while (!isEnd) {
if (!isSuspended) {
// Odczytanie danych ze standardowego strumienia wejściowego
if (fgets(buffer, MAX_SIZE, stdin) != NULL) {
// Przekazanie danych do procesu 2 za pomocą łącza
if (write(fd[1], buffer, strlen(buffer)) == -1) {
perror("write");
exit(EXIT_FAILURE);
}
}
}
else {
printf("\nP1: Wstrzymuje dzialanie...");
pause();
}
}
close(fd[1]); // Zamknięcie strony zapisu łącza
printf("\nP1: Koncze dzialanie...");
exit(EXIT_SUCCESS);
}
// Utworzenie procesu 2
if ((pid2 = fork()) == -1) {
perror("fork");
exit(EXIT_FAILURE);
}
else if (pid2 == 0) {
// Kod procesu 2
int wordCount = 0;
char buffer[MAX_SIZE] = {};
close(fd[1]); // Zamknięcie strony zapisu łącza
int bytesRead = 0;
while (!isEnd) {
if (!isSuspended) {
wordCount = 0; //Liczy słowa w każdej linii przekazanej przez proces 1 oddzielnie. wystarczy usunąc żeby liczyło wszsytkie wystąpienia.
// Odczytanie danych z łącza
bytesRead = read(fd[0], buffer, MAX_SIZE);
if (bytesRead == -1) {
perror("read");
exit(EXIT_FAILURE);
}
else if (bytesRead == 0) {
//break; // Koniec danych
}
//printf("P2: Liczba bajtów: %d", bytesRead);
// Zliczenie liczby słów w danych
for (int i = 0; i < bytesRead; i++) {
if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t') {
wordCount++;
}
}
//printf("\nP2: liczba slow: %d\n", wordCount);
// Przekazanie danych i liczby słów do procesu 3 za pomocą kolejki komunikatów
msg.mtype = 1;
strncpy(msg.mtext, buffer, bytesRead);
msg.mtext[bytesRead] = '\0'; // Upewnienie się, że wiersz kończy się zerem
msg.words = wordCount;
int msgLen = MAX_SIZE + sizeof(int);
if (msgsnd(msgid, &msg, msgLen, 0) == -1) {
perror("msgsnd");
exit(EXIT_FAILURE);
}
}
else {
printf("\nP2: Wstrzymuje dzialanie...");
pause();
}
}
close(fd[0]); // Zamknięcie strony odczytu łącza
printf("\nP2: Koncze dzialanie...");
exit(EXIT_SUCCESS);
}
// Utworzenie procesu 3
if ((pid3 = fork()) == -1) {
perror("fork");
exit(EXIT_FAILURE);
}
else if (pid3 == 0) {
// Kod procesu 3
close(fd[0]); // Zamknięcie strony odczytu łącza
close(fd[1]); // Zamknięcie strony zapisu łącza
while (!isEnd) {
if (!isSuspended) {
// Odczytanie danych i liczby słów z kolejki komunikatów
if (msgrcv(msgid, &msg, MAX_SIZE + sizeof(int), 1, 0) == -1) {
if (errno == EINTR) break;
else {
perror("msgrcv");
exit(EXIT_FAILURE);
}
}
// Wypisanie danych i liczby słów na ekran
int msgLen = strlen(msg.mtext) + 1;
//printf("P3: Tekst: %s\nP3: liczba slow: %d\n", msg.mtext, *(int*)(&msg.mtext[msgLen]));
printf("\nP3: Tekst: %sP3: liczba slow: %d\n", msg.mtext, msg.words);
}
else {
printf("\nP3: Wstrzymuje dzialanie...");
pause();
}
}
printf("\nP3: Koncze dzialanie...");
exit(EXIT_SUCCESS);
}
// Proces macierzysty czeka na zakończenie procesów potomnych
waitpid(pid1, &status, 0);
waitpid(pid2, &status, 0);
waitpid(pid3, &status, 0);
// Usunięcie kolejki komunikatów
if (msgctl(msgid, IPC_RMID, NULL) == -1) {
perror("msgctl");
exit(EXIT_FAILURE);
}
return 0;
}