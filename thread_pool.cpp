#include "thread_pool.h"
#include <cstdlib>
#include <unistd.h>
class task
{
public:
    int a;
    void process()
    {
        srand(time(NULL));
        printf("i am the %d task\n",a);
        sleep(1000);
        printf("i have done!\n");
    }
};
int main()
{
    threadpool<task> pool;
    task ts[100];
    for(int i = 0;i < 100;i++)
    {
        ts[i].a = i;
        pool.append(ts+i);
    }
    return 0;
}
