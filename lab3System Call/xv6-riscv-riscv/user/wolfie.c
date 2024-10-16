#include "types.h"
#include "stat.h"
#include "user.h"
void show_ans(int ans)
{
if(ans == -1)
{
printf(0, "the buffer is not enough\n");
}
else
{
printf(0, "the string length is %d\n", ans);
}
printf(0, "\n");
return;
}
int main(int argc, char* argv[])
{
char buf_one[1000] = {0};
printf(0, "create a buffer which size is 1000\n");
int ans_one = wolfie(buf_one, 1000);
show_ans(ans_one);
printf(0, "%s", buf_one);
char buf_two[10] = {0};
printf(0, "create a buffer which size is 10\n");
int ans_two = wolfie(buf_two, 10);
show_ans(ans_two);
return 0;
}