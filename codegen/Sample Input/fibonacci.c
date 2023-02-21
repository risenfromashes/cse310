int f[100];

int fib(int i){
  if(i <= 1) {
    return 1;
  }
  if(f[i]) {
    return f[i];
  } else {
    f[i] = fib(i-1) + fib(i-2);
    return f[i];
  }
}

int main(){
  println(fib(10));
}
