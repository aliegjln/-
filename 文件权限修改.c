#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
int main()
{
    chmod("a.out",0755);//失败返回－１　成功返回０；
    printf("%s\n",__FILE__);
    return 0;
}
/*
十进制，八进制问题。
mode参数
修改自己的问题(写自己文件名)
*/
