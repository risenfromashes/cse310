int a[5];

void sort() {
    int i, j;
    for (i = 0; i < 5; i++) {
        j = i + 1;
        while (j < 5) {
            if (a[i] > a[j]) {
                int temp;
                temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
            j++;
        }
    }
}

int main() {
    a[0] = 69;
    a[1] = 7;
    a[2] = 23;
    a[3] = 9;
    a[4] = 1;
    sort();
    int i;
    for (i = 0; i < 5; i++) {
        int l;
        l = a[i];
        println(l);
    }
}

