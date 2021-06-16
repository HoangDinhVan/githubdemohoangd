#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

using namespace std;

sharing memory between processes
int main() {
    pthread_mutexattr_t attributes;
    pthread_mutexattr_init(&attributes);
    pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);

    int handle = shm_open("/shm", O_CREAT | O_RDWR, 0777);
    ftruncate(handle, 2048 * sizeof(int));
    char *memory = (char*)mmap(0, 2048 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);

    //mutex share -> set mutex address at begin of the shared memory
    pthread_mutex_t *mutex = (pthread_mutex_t*) memory;
    pthread_mutex_init(mutex, &attributes);
    pthread_mutexattr_destroy(&attributes);

    cout<< "Memory address: "<< &memory<< endl;


    // variable share -> at right after muxtex
    int* count = (int*)(memory + sizeof(pthread_mutex_t));
    *count = 0;

    cout<< "count address: "<< &count<< endl;;

    cout << "Initial count = "<< *count<< endl;

    int value_returned_from_child = 0;
    pid_t pid = fork();

    if(pid == 0){
        pthread_mutex_lock(mutex);
        (*count)++;
        cout<< "Child Process increased the count to "<< *count << endl;
        pthread_mutex_unlock(mutex);
        value_returned_from_child = 99;
    }

    else{
        int status;
        cout<< "pid of child process: "<< pid<< endl;
        // waiting for the child process to finish
        waitpid(pid, &status, 0);
        value_returned_from_child = WEXITSTATUS(status);
        cout<< "value return from child = "<< value_returned_from_child << endl;
        pthread_mutex_lock(mutex);
        (*count)++;
        cout<< "Parent process increased the count to "<< (*count)<< endl;
        pthread_mutex_unlock(mutex);
    }

    munmap(memory, 2048*sizeof(int));
    shm_unlink("/shm");
    return value_returned_from_child;

}

Sharing semaphores between processes
sem_post: increase couter -> 1, sem_wait: wait counter > 0 and decrease by 1; sem_trywait: decrease counter by 1

int main(){
    sem_t *sem_id = sem_open("/sema", O_CREAT, 0777, 0);

    pid_t pid = fork();
    if(pid == 0){
        cout << "Child: done, release semaphore"<< endl;
         sem_post(sem_id);
        sleep(2);
        if(sem_wait(sem_id) < 0)
            cout<< "Child: [sem_wait] failed" << endl;
         cout<< "Child: wait semaphore Done"<< endl;

        sem_close(sem_id);
    } else {
        int status;
        if(sem_wait(sem_id) < 0)
            cout<< "Parent: [sem_wait] failed" << endl;
        cout<< "Parent: child printed" << endl;
        sem_post(sem_id);
        sem_close(sem_id);
        sem_unlink("/sema");
        wait(&status);
        cout<<"Parent done" << endl;
    }
    return 0;
}