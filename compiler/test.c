int main() {
    private float a[14], b[14];
    private int cmp_result[14];
    smcinput(a, 1, 14);
    smcinput(b, 1, 14);

    cmp_result[0] = a[0] < b[0];
    cmp_result[1] = a[1] < b[1];
    cmp_result[2] = a[2] < b[2];
    cmp_result[3] = a[3] < b[3];
    cmp_result[4] = a[4] < b[4];
    cmp_result[5] = a[5] < b[5];
    cmp_result[6] = a[6] < b[6];
    cmp_result[7] = a[7] < b[7];
    cmp_result[8] = a[8] < b[8];
    cmp_result[9] = a[9] < b[9];
    cmp_result[10] = a[10] < b[10];
    cmp_result[11] = a[11] < b[11];
    cmp_result[12] = a[12] < b[12];
    cmp_result[13] = a[13] < b[13];

    smcoutput(cmp_result, 1, 14);
}
