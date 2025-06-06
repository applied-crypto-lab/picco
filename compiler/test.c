int main() {
    private float a[14], b[14];
    private int cmp_result[14];

    smcinput(a, 1, 14);    // Input: party 1 supplies 14 private floats for a
    smcinput(b, 1, 14);    // Input: party 1 supplies 14 private floats for b

    // Secure element-wise comparison
    for (int i = 0; i < 14; ++i) {
        private int tmp;
        tmp = a[i] < b[i];
        cmp_result[i] = tmp;
    }
    
    smcoutput(cmp_result, 1, 14); // Output results to party 1

    return 0;
}
