#define N 14

int main() {
    privfloat a[N], b[N];
    privint cmp_result[N];

    // Read N values for each array from party 1
    smcinput(a, 1, N);
    smcinput(b, 1, N);

    // Compare a[i] < b[i] for all i
    for (int i = 0; i < N; ++i) {
        cmp_result[i] = (a[i] < b[i]);
    }

    // Output all results to party 1
    smcoutput(cmp_result, 1, N);

    return 0;
}
