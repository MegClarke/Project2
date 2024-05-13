#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  u32 remaining_time;
  u32 start_exec_time;
  u32 waiting_time;
  u32 response_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */

  int t = 0;
  int finished = 0;
  struct process* executing = NULL;
  int been_executing = 0;

  while(finished < size){
    //add to queue if time == arrival time;
    for(int i = 0; i < size; i++){
      if (data[i].arrival_time == t){
        data[i].remaining_time = data[i].burst_time;
        struct process* new_process = malloc(sizeof(struct process));
        *new_process = data[i];
        TAILQ_INSERT_TAIL(&list, new_process, pointers);
      }
    }
    
    //currently executing something
    if(executing != NULL){
      executing->remaining_time--;
      been_executing++;
      if(executing->remaining_time <= 0){
        finished++;
        executing->waiting_time = t - executing->arrival_time - executing->burst_time;
        total_waiting_time += t - executing->arrival_time - executing->burst_time;
        free(executing);
        executing = NULL;
      }
      else if(been_executing == quantum_length){
        TAILQ_INSERT_TAIL(&list, executing, pointers);
        executing = NULL;
      }
    }

    //make front of queue -> executing
    if(executing == NULL){
      been_executing = 0;
      struct process* first = TAILQ_FIRST(&list);
      if (first) {
        //printf("First item value: %d\n", first->value);
        executing = first;
        TAILQ_REMOVE(&list, first, pointers);
        if (executing->remaining_time == executing->burst_time) {
          executing->start_exec_time = t;
          executing->response_time = t - executing->arrival_time;
          total_response_time += t - executing->arrival_time;
        }
      }
    } 
    /* PRINTING FOR DEBUGGING
    printf("Current time: %d\n", t);
    if(executing == NULL){
      printf("Executing: %s\n", "NONE");
    }
    else{
      printf("Executing: %u\n", executing->pid);
    }
    
    struct process* curr_process;
    TAILQ_FOREACH(curr_process, &list, pointers) {
      printf("Process ID: %u\n", curr_process->pid);
    }
    */

    t++;                  
  }

  //don't forget to free the process when done

  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
