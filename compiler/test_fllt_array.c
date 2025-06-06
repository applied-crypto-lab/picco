int main() {
    private float a[14], b[14];
    private int cmp_result[14];

    smcinput(a, 1, 14);
    smcinput(b, 1, 14);

    for (int i = 0; i < 14; ++i) {
        cmp_result[i] = (a[i] < b[i]);
    }

    smcoutput(cmp_result, 1, 14);

    return 0;
}
