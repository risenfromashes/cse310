int arr[6];
int show(int n)
{
    if(n>=6)
    {
        return -1;
    }
    show(n+1);
    int x;
    x=arr[n];
    println(x);
    return -1;
}
int main()
{
    int j;
    for(j=0;j<6;j++)
    {
        arr[j]=j*j;
    }
    show(0);
    return 0;
}