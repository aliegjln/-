#include<stdio.h>
#define swip(a,b) {(a)=(a)+(b);(b)=(a)-(b);(a)=(a)-(b);}
#define n 10
int Bubble_Sort(int a[n]);

/*————————————————冒泡——————————————————
优点:,简单，稳定
缺点：交换次数多，效率低，不适合多数据
*/
void BubbleSort1(int a[])
{
    int i,j;
    int flag;//优化1;如果某一轮两两比较中没有任何元素交换，这说明已经都排好序了，故可以使用一个flag做标记。（减少比较次数）
    for (i=0;i<n;i++)
    {
        flag=1;
        for (j=0;j<n-1-i;j++)
            if (a[j]<a[j+1])
            {
                flag=0;
                swip(a[j],a[j+1]);
            }
        if(flag)
            break;
    }
}
int Bubble_Sort2(int a[])
{
    int i,j;
    int flag;//优化2:用一个变量记录下最后一个发生交换的位置，后面没有发生交换的已经有序 所以可以用这个值来作为下一次比较结束的位置 ，故下一轮的可以提前结束。（减少比较次数）
    int time;
    flag=n-1;
    for(i=0;i<n-1;i++)
    {
        for(j=0;j<flag;j++)
            if(a[j]>a[j+1])
            {
                swip(a[j],a[j+1]);
                time=j;
            }
        flag=time;
    }
}
int Bubble_Sort3(int a[])//双向冒泡（鸡尾酒排序）每次从前到后，再从后到前，每一次就将两侧大小全冒泡出来（排序思路改变）
{
    int i,j;
    int flag_r,flag_l;//这个玩意相当于上面的优化2，记录左右大小已经有序的位置
    int time_r,time_l;
    flag_r=0;
    flag_l=n-1;
    int flag;//这个相当于上面的优化1,如果没有交换就说明排好了
    while(flag_r<flag_l)
    {
        flag=1;
        for(j=flag_r;j<flag_l;j++)
            if(a[j]>a[j+1])
            {
                time_l=j;
                swip(a[j],a[j+1]);
                flag=0;
            }
        flag_l=time_l;
        for(j=flag_l;j>flag_r;j--)
            if(a[j]<a[j-1])
            {
                time_r=j;
                swip(a[j],a[j-1]);
                flag=0;
            }
        flag_r=time_r;
        if(flag)
            break;
    }
}
/*-------------选择排序-------------
优点:简单，小范围数据速度快
缺点:多数据太耗时，不稳定
*/
int Selection_sort(int a[])//一般的选择排序
{
    int i,j;
    for(i=0;i<n;i++)
    {
        for(j=i;j<n;j++)
        {
            if(a[i]>a[j])
                swip(a[i],a[j]);
        }
    }
}
int Selection_sort1(int a[])//改进，双向选择排序
{
    int i,j;
    int flag_r,flag_l;
    flag_r=0;
    flag_l=n-1;
    while(flag_r<flag_l)
    {
        for(j=flag_r;j<flag_l;j++)
        {
            if(a[flag_r]>a[j])
                swip(a[flag_r],a[j]);
        }
        flag_r++;
        for(j=flag_l;j<flag_r;j--)
        {
            if(a[flag_l]<a[j])
                swip(a[flag_l],a[j]);
        }
        flag_l--;
    }
}
int main()
{
    int i;
    int a[10]={7,5,9,3,2,0,4,1,8,6};
    Selection_sort1(a);
    for(i=0;i<n;i++)
        printf("%d ",a[i]);

}








