int is_prime(int p){
	if(p <= 1){
		return 0;
	}

	int i;
	for(i = 2; i * i <= p; i++){
		if(p % i == 0){
			return 0;
		}
	}

	return 1;
}

int main(){
	int i;
	for(i = 1; i < 100; i++){
		if(is_prime(i)){
			println(i);
		}
	}
	return 0;
}
