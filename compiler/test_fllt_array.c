int main() {
    private int a[2], b[2], cmp_result[2];
    smcinput(a, 1, 2);
    smcinput(b, 1, 2);

    for (int i = 0; i < 2; ++i)
        cmp_result[i] = a[i] < b[i];

    smcoutput(cmp_result, 1, 2);

}
