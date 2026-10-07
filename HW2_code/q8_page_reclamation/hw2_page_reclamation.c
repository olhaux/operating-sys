// usage: ./hw2_page_reclamation N M
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define REF_LEN 10000
#define WATERMARK_PCT 70 // active list limit, in % of N

enum { ACTIVE = 1, INACTIVE = 2 };

typedef struct page {
     int page_id;
     int reference_bit;
     struct page *next;
     // other auxiliary
     struct page *prev;
     int list; // ACTIVE, INACTIVE or 0 before the first reference
     long total_referenced;
     long true_accesses; // real number of accesses
} Node;

typedef struct {
     Node *head; // front, least recently used
     Node *tail; // rear, most recently used
     int size;
} List;

// create an active list
static List active_list = { NULL, NULL, 0 };

// create an inactive list
static List inactive_list = { NULL, NULL, 0 };

static Node *pages; // pages[i] is page i
static int *ref_string;
static int N, M, watermark;
static int player_done = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// list helpers, only called with the lock held
static void list_remove(List *l, Node *p) {
     if (p->prev) p->prev->next = p->next; else l->head = p->next;
     if (p->next) p->next->prev = p->prev; else l->tail = p->prev;
     p->next = p->prev = NULL;
     l->size--;
}

static void list_append(List *l, Node *p) {
     p->next = NULL;
     p->prev = l->tail;
     if (l->tail) l->tail->next = p; else l->head = p;
     l->tail = p;
     l->size++;
}

static void list_print(const List *l) {
     for (Node *p = l->head; p; p = p->next)
          printf("%d%s", p->page_id, p->next ? ", " : "");
     printf("\n");
}

void *player_thread_func(void *arg) {
     for (int i = 0; i < REF_LEN; i++) {
          pthread_mutex_lock(&lock);
          Node *p = &pages[ref_string[i]];
          p->reference_bit = 1;
          p->true_accesses++;

          // move the page to the rear of the active list
          if (p->list == ACTIVE)   list_remove(&active_list, p);
          if (p->list == INACTIVE) list_remove(&inactive_list, p);
          list_append(&active_list, p);
          p->list = ACTIVE;

          // over the watermark, move pages from the front to the inactive list
          while (active_list.size > watermark) {
               Node *victim = active_list.head;
               list_remove(&active_list, victim);
               list_append(&inactive_list, victim);
               victim->list = INACTIVE;
          }
          pthread_mutex_unlock(&lock);

          usleep(10);
     }
     pthread_mutex_lock(&lock);
     player_done = 1;
     pthread_mutex_unlock(&lock);
     pthread_exit(0);
}

void *checker_thread_func(void *arg) {
     int done = 0;
     while (!done) {
          usleep(M);
          pthread_mutex_lock(&lock);
          done = player_done; // one last pass after the player is done
          for (Node *p = active_list.head; p; p = p->next) {
               if (p->reference_bit) {
                    p->total_referenced++;
                    p->reference_bit = 0;
               }
          }
          pthread_mutex_unlock(&lock);
     }
     pthread_exit(0);
}

int main(int argc, char *argv[])
{
     N = atoi(argv[1]);
     M = atoi(argv[2]);
     watermark = N * WATERMARK_PCT / 100;

     pages = calloc(N, sizeof(Node));
     ref_string = malloc(REF_LEN * sizeof(int));
     if (!pages || !ref_string) { perror("alloc"); return 1; }
     for (int i = 0; i < N; i++) pages[i].page_id = i;

     // create a random reference string
     srand(time(NULL) ^ getpid());
     for (int i = 0; i < REF_LEN; i++) ref_string[i] = rand() % N;

     /* Create two workers */
     pthread_t player;
     pthread_t checker;

     pthread_create(&player, NULL, player_thread_func, NULL);
     pthread_create(&checker, NULL, checker_thread_func, NULL);

     pthread_join(player, NULL);
     pthread_join(checker, NULL);

     long sum_checker = 0, sum_true = 0;
     printf("Page_Id, Total_Referenced\n");
     //Print out the statistics of page references
     for (int i = 0; i < N; i++) {
          printf("%d, %ld\n", i, pages[i].total_referenced);
          sum_checker += pages[i].total_referenced;
          sum_true += pages[i].true_accesses;
     }

     printf("Pages in active list: ");
     //Print out the list of pages in active list
     list_print(&active_list);

     printf("Pages in inactive list: ");
     //Print out the list of pages in inactive list
     list_print(&inactive_list);

     // checks we use in 8.1
     printf("\n[check] N=%d M=%d watermark=%d\n", N, M, watermark);
     printf("[check] active=%d inactive=%d (sum %d)\n",
            active_list.size, inactive_list.size,
            active_list.size + inactive_list.size);
     printf("[check] accesses seen by checker = %ld, real accesses = %ld (%.1f%%)\n",
            sum_checker, sum_true, 100.0 * sum_checker / sum_true);

     /*free up resources properly */
     free(pages);
     free(ref_string);
     pthread_mutex_destroy(&lock);
     return 0;
}
