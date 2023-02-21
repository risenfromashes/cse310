int binexp(int a, int b){
	if(b == 0) {
		return 1;
	}
	int t;
	t = binexp(a, b/2);
	t = t * t;
	if(b % 2){
		return a * t;
	} 
	else {
		return t;
	}
}

int main(){
	println(binexp(2, 12));
}