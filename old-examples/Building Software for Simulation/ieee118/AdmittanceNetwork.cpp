#include "AdmittanceNetwork.h"
#include <cassert>
#include <iostream>


// LAPACK function for solving a complex Ax = b
extern "C" {
void zgetrs_(char*, int*, int*, double*, int*, int*, double*, int*, int*);
void zgetrf_(int*, int*, double*, int*, int*, int*);
};

AdmittanceNetwork::AdmittanceNetwork(int N)
    : N(N),
      Y(new double[2 * N * N]),
      LUY(new double[2 * N * N]),
      dirty(false),
      iv_lapack(new double[2 * N]),
      ipiv_lapack(new int[N]) {
    for (int i = 0; i < 2 * N * N; i++) {
        Y[i] = 0.0;
    }
}

void AdmittanceNetwork::set(int i, int j, Complex y) {
    dirty = true;
    int real_part = i * 2 * N + 2 * j;
    int imag_part = real_part + 1;
    Y[real_part] = real(y);
    Y[imag_part] = imag(y);
}

Complex AdmittanceNetwork::get(int i, int j) const {
    int real_part = i * 2 * N + 2 * j;
    int imag_part = real_part + 1;
    return Complex(Y[real_part], Y[imag_part]);
}

void AdmittanceNetwork::add_line(int i, int j, Complex y) {
    set(i, i, get(i, i) + y);
    set(j, j, get(j, j) + y);
    set(i, j, get(i, j) - y);
    set(j, i, get(j, i) - y);
}

void AdmittanceNetwork::add_self(int i, Complex y) {
    set(i, i, get(i, i) + y);
}

void AdmittanceNetwork::solve_for_current(Complex const* V, Complex* I) {
    // Calculate YV=I
    for (int i = 0; i < N; i++) {
        I[i] = Complex(0.0, 0.0);
        for (int j = 0; j < N; j++) {
            I[i] += get(j, i) * V[j];
        }
    }
}

void AdmittanceNetwork::solve_for_voltage(Complex const* I, Complex* V) {
    int i, info, SIZE = N, lda = N, ldb = N, nrhs = 1;
    char type = 'N';
    // Do the LU decomposition if necessary
    if (dirty) {
        dirty = false;
        for (int i = 0; i < 2 * N * N; i++) {
            LUY[i] = Y[i];
        }
        zgetrf_(&SIZE, &SIZE, LUY, &lda, ipiv_lapack, &info);
        assert(info == 0);
    }
    // Put the current into LAPACK form
    for (i = 0; i < N; i++) {
        iv_lapack[2 * i] = real(I[i]);
        iv_lapack[2 * i + 1] = imag(I[i]);
    }
    // Solve for the voltage
    zgetrs_(&type, &SIZE, &nrhs, LUY, &lda, ipiv_lapack, iv_lapack, &ldb,
            &info);
    assert(info == 0);
    // Get the current into the complex return vector
    for (i = 0; i < N; i++) {
        V[i] = Complex(iv_lapack[2 * i], iv_lapack[2 * i + 1]);
    }
}

AdmittanceNetwork::~AdmittanceNetwork() {
    delete[] Y;
    delete[] LUY;
    delete[] iv_lapack;
    delete[] ipiv_lapack;
}
