#define N 14

int main() {
    private float a[N], b[N];
    private int cmp_result[N];

    smcinput(a, 1, N);
    smcinput(b, 1, N);

    for (int i = 0; i < N; ++i) {
        cmp_result[i] = (a[i] < b[i]);
    }

    smcoutput(cmp_result, 1, N);

    return 0;
}