int main()
{
    int d[20];
    d[0]=1;
    d[1]=2;
    d[2]=3;
    d[4]=4;
    d[d[2]+d[1]-d[0]]=50+d[4];
    int x;
    x=d[4];
    println(x);
    return 0;
}