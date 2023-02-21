int main(){
	int i;
	for(i = 0; i < 100; i++){
		if(i % 2 == 0){
			continue;
		}
		println(i);
	}
}
