#include<stdio.h>
const int c[2][2] = {{1,2},{3,4}};
void f2(int a[]) {
    printf("a[0]: %d\n",a[0]);
}

void f1(int a[][c[0][1]]) {
    f2(a[0]);
 // printf
}

int main(){
    int tmp[2][2] = {2,3,123,43};
    f1(tmp);
}
