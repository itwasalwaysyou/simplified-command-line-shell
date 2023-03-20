#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "job_list.h"
#include "string_vector.h"
#include "swish_funcs.h"

#define MAX_ARGS 10

int tokenize(char *s, strvec_t *tokens) {
    // TODO Task 0: Tokenize string s
    // Assume each token is separated by a single space (" ")
    // Use the strtok() function to accomplish this
    // Add each token to the 'tokens' parameter (a string vector)
    // Return 0 on success, -1 on error
    char d[]=" ";
    char *token =strtok(s,d);
    while(token!=NULL){
        strvec_add(tokens, token);
        token=strtok(NULL,d);
    
    }
    
    return 0;
}

// if not arrow: return -1
int redirect_option(char* curr) {
    if(strcmp(curr,"<")==0){
        return 0; // redirect stdin
    }else if(strcmp(curr,">")==0){
        return 1; // redirect std out
    }else if(strcmp(curr,">>")==0){
        return 2; // redirect std out to the end
    }
    else {
        return -1;
    }


}

int run_command(strvec_t *tokens) {
    // TODO Task 2: Execute the specified program (token 0) with the
    // specified command-line arguments
    // THIS FUNCTION SHOULD BE CALLED FROM A CHILD OF THE MAIN SHELL PROCESS
    // Hint: Build a string array from the 'tokens' vector and pass this into execvp()
    // Another Hint: You have a guarantee of the longest possible needed array, so you
    // won't have to use malloc.

    // TODO Task 3: Extend this function to perform output redirection before exec()'ing
    // Check for '<' (redirect input), '>' (redirect output), '>>' (redirect and append output)
    // entries inside of 'tokens' (the strvec_find() function will do this for you)
    // Open the necessary file for reading (<), writing (>), or appending (>>)
    // Use dup2() to redirect stdin (<), stdout (> or >>)
    // DO NOT pass redirection operators and file names to exec()'d program
    // E.g., "ls -l > out.txt" should be exec()'d with strings "ls", "-l", NULL

    // TODO Task 4: You need to do two items of setup before exec()'ing
    // 1. Restore the signal handlers for SIGTTOU and SIGTTIN to their defaults.
    // The code in main() within swish.c sets these handlers to the SIG_IGN value.
    // Adapt this code to use sigaction() to set the handlers to the SIG_DFL value.
    // 2. Change the process group of this process (a child of the main shell).
    // Call getpid() to get its process ID then call setpgid() and use this process
    // ID as the value for the new process group ID

    // Not reachable after a successful exec(), but retain here to keep compiler happy
    struct sigaction sac;
    sac.sa_handler = SIG_DFL;

    if (sigfillset(&sac.sa_mask) == -1) {

        perror("sigfillset");

        return 1;

    }

    sigaddset(&sac.sa_mask,SIGTTIN);

    sigaddset(&sac.sa_mask,SIGTTOU);

    sac.sa_flags = 0;

    if (sigaction(SIGTTIN, &sac, NULL) == -1 || sigaction(SIGTTOU, &sac, NULL) == -1) {

        perror("sigaction");

        return 1;

    }


    char *array[MAX_ARGS];


    int i;
    for(i=0; i< tokens->length;i++){   
        char * curr=strvec_get(tokens,i); 
        if (redirect_option(curr) == -1){
            array[i]=strvec_get(tokens,i);
        }
        else{
            break;
        }
    }

    array[i]=NULL;

    if(i!= (tokens->length-1)){
        int cur_mode;
        for(;i<tokens->length;i++){        
            char * curr=strvec_get(tokens,i);
            if(redirect_option(curr) != -1){ 
   

                cur_mode = redirect_option(curr);


            }else{ 
                
                // file name
                char *file_name = curr; 

                if (cur_mode == 0) {   // eg:  cat < gatsby.txt
                    int file_in = open(file_name, O_RDONLY, 400|200|100);
                    if(file_in==-1){
                        perror("Failed to open input file");
                        return -1;
                    }
                    if (dup2(file_in, STDIN_FILENO) == -1) {
                        perror("dup2 failed");
                        close(file_in);
                    }
                    close(file_in);


                }
            
        
                if (cur_mode == 1) {    // eg : ls -l > out.txt
                    int file_out = open(file_name, O_CREAT|O_TRUNC|O_WRONLY, 400|200|100); 
                    if(file_out==-1){

                        perror("write");
                        return -1;
                    }
                    if (dup2(file_out,STDOUT_FILENO) == -1) {
                        perror("dup2 failed");
                        close(file_out);
                    }
               }

               if (cur_mode == 2) {   //eg : ls -l >> out.txt
                    int file_append = open(file_name, O_CREAT|O_APPEND|O_WRONLY, 400|200|100);
                    if(file_append==-1){
                        perror("append");
                        return -1;
                    }
                    if (dup2(file_append,STDOUT_FILENO) == -1) {
                        perror("dup2 failed");
                        close(file_append);
                    }
               }
                

            }

        }
    }



    const char*first_token= strvec_get(tokens,0);
    execvp(first_token,array);
    perror("exec");


    return -1;


    }


int resume_job(strvec_t *tokens, job_list_t *jobs, int is_foreground) {
    // TODO Task 5: Implement the ability to resume stopped jobs in the foreground
    // 1. Look up the relevant job information (in a job_t) from the jobs list
    //    using the index supplied by the user (in tokens index 1)
    //    Feel free to use sscanf() or atoi() to convert this string to an int
    // 2. Call tcsetpgrp(STDIN_FILENO, <job_pid>) where job_pid is the job's process ID
    // 3. Send the process the SIGCONT signal with the kill() system call
    // 4. Use the same waitpid() logic as in main -- dont' forget WUNTRACED
    // 5. If the job has terminated (not stopped), remove it from the 'jobs' list
    // 6. Call tcsetpgrp(STDIN_FILENO, <shell_pid>). shell_pid is the *current*
    //    process's pid, since we call this function from the main shell process
    
    int index= atoi(strvec_get(tokens,1));
    job_t *job = job_list_get(jobs,index);

    if(job==NULL){

        fprintf(stderr, "Job index out of bounds\n");
        return -1;
    }

    if(is_foreground==1){
        //job pid (process) --> foreground         
        if(tcsetpgrp(STDIN_FILENO, job->pid)==-1){ 
                perror("tcsetpgrp");
                return -1;
        }

        //resume the job with STOPPED status
        killpg(job->pid, SIGCONT);

        int stat;
        waitpid(job->pid,&stat,WUNTRACED); //WUNTRACED-- detect if it has stopped from a signal



        // if(WIFSTOPPED(stat)!=0){
        // }

        // SHELL --> foreground
        if(tcsetpgrp(STDIN_FILENO, getpgrp())==-1){
            perror("tcsetpgrp");
            return -1;

        } 
        job_list_remove(jobs,index);


    }else{
        //resume the job with STOPPED status
        killpg(job->pid, SIGCONT);
        //change the status to BACKGROUND
        job->status=JOB_BACKGROUND;

    }

    

    // TODO Task 6: Implement the ability to resume stopped jobs in the background.
    // This really just means omitting some of the steps used to resume a job in the foreground:
    // 1. DO NOT call tcsetpgrp() to manipulate foreground/background terminal process group
    // 2. DO NOT call waitpid() to wait on the job
    // 3. Make sure to modify the 'status' field of the relevant job list entry to JOB_BACKGROUND
    //    (as it was JOB_STOPPED before this)

    return 0;
}

int await_background_job(strvec_t *tokens, job_list_t *jobs) {
    // TODO Task 6: Wait for a specific job to stop or terminate
    // 1. Look up the relevant job information (in a job_t) from the jobs list
    //    using the index supplied by the user (in tokens index 1)
    // 2. Make sure the job's status is JOB_BACKGROUND (no sense waiting for a stopped job)
    // 3. Use waitpid() to wait for the job to terminate, as you have in resume_job() and main().
    // 4. If the process terminates (is not stopped by a signal) remove it from the jobs list
    int index= atoi(strvec_get(tokens,1));
    job_t *job = job_list_get(jobs,index);

    if (job->status==JOB_STOPPED){
        fprintf(stderr, "Job index is for stopped process not background process\n");
        return -1;
    }else{
        int stat;
        waitpid(job->pid,&stat,WUNTRACED); //WUNTRACED-- detect if it has stopped from a signal


        if(!WIFSTOPPED(stat)){ //if the job process not stopped by a signal
                job_list_remove(jobs,index);

        }else{
            job->pid=JOB_STOPPED;
        }

    }


    return 0;
}

int await_all_background_jobs(job_list_t *jobs) {
    // TODO Task 6: Wait for all background jobs to stop or terminate
    // 1. Iterate through the jobs list, ignoring any stopped jobs
    // 2. For a background job, call waitpid() with WUNTRACED.
    // 3. If the job has stopped (check with WIFSTOPPED), change its
    //    status to JOB_STOPPED. If the job has terminated, do nothing until the
    //    next step (don't attempt to remove it while iterating through the list).
    // 4. Remove all background jobs (which have all just terminated) from jobs list.
    //    Use the job_list_remove_by_status() function.
    job_t *curr= job_list_get(jobs,0);
    
    while(curr->next!=NULL){
        if(curr->status==JOB_STOPPED){
            curr=curr->next;
        }else{
            int stat;
            waitpid(curr->pid,&stat,WUNTRACED); //WUNTRACED-- detect if it has stopped from a signal


            if(WIFSTOPPED(stat)){ //if the job process not stopped by a signal
                curr->pid=JOB_STOPPED;

            }
            curr=curr->next;
        }

    }
    job_list_remove_by_status(jobs, JOB_BACKGROUND);

    return 0;
}
