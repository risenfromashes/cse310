int a[5];

void sort() {
    int i, j;
    int b[5];
    for (i = 0; i < 5; i++) {
        b[i] = a[i];
    }
    for (i = 0; i < 5; i++) {
        j = i + 1;
        while (j < 5) {
            if (b[i] > b[j]) {
                int temp;
                temp = b[i];
                b[i] = b[j];
                b[j] = temp;
            }
            j++;
        }
    }
    for (i = 0; i < 5; i++) {
        a[i] = b[i];
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
        println(a[i]);
    }
}

