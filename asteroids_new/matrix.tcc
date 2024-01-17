#include <cassert>
#include <iostream>

template <class FLOAT_TYPE, size_t N>
SquareMatrix<FLOAT_TYPE, N>::SquareMatrix(std::initializer_list< Vector<FLOAT_TYPE, N > > values) {
    auto iterator = values.begin();
    for (size_t i = 0u; i < N; i++) {
        if (iterator != values.end()) {
            matrix[i] = *iterator++;
        } else {
            Vector<FLOAT_TYPE, N> zeroVector;
            for(size_t j = 0; j < N; j++){
                zeroVector.vector[j] = 0;
            }
            matrix[i] = (i > 0 ? matrix[i - 1] : zeroVector);
        }
    }
}

//default constructor
template <class FLOAT_TYPE, std::size_t N>
SquareMatrix<FLOAT_TYPE, N>::SquareMatrix() {}

// returns reference to the i-th column vector
//Vector<FLOAT, N> & operator[](std::size_t i);
template <class FLOAT_TYPE, std::size_t N>
Vector<FLOAT_TYPE, N> & SquareMatrix<FLOAT_TYPE, N>::operator[](std::size_t i) {
    return matrix[i];
}

// returns i-th column vector
//Vector<FLOAT, N> operator[](std::size_t i) const;
template <class FLOAT_TYPE, std::size_t N>
Vector<FLOAT_TYPE, N> SquareMatrix<FLOAT_TYPE, N>::operator[](std::size_t i) const {
    return matrix[i];
}

// returns the value at the given row and column
//FLOAT at(size_t row, size_t column) const;
template <class FLOAT_TYPE, std::size_t N>
FLOAT_TYPE SquareMatrix<FLOAT_TYPE, N>::at(std::size_t row, std::size_t column) const {
    return matrix[column][row];
}

// returns the reference value at the given row and column
//FLOAT & at(size_t row, size_t column);
template <class FLOAT_TYPE, std::size_t N>
FLOAT_TYPE & SquareMatrix<FLOAT_TYPE, N>::at(std::size_t row, std::size_t column) {
    return matrix[column][row];
}

// returns the producut of this SquareMatrix and the given vector
//Vector<FLOAT,N> operator*(const Vector<FLOAT,N> vector);
template <class FLOAT_TYPE, std::size_t N>
Vector<FLOAT_TYPE, N> SquareMatrix<FLOAT_TYPE, N>::operator*(const Vector<FLOAT_TYPE, N> vector) {
    //initialize result vector with 0
    Vector<FLOAT_TYPE, N> result;
    for(size_t i = 0u; i < N; i++){
        result[i] = 0.0;
    }
    for (size_t i = 0u; i < N; i++) {
        for (size_t j = 0u; j < N; j++) {
            result[i] += matrix[j][i] * vector[j];
        }
    }
    return result;
}


//  returns the product of two square matrices
//template <class F, size_t K>
//friend SquareMatrix<F, K> operator*(const SquareMatrix<F, K> factor1, const SquareMatrix<F, K> factor2);
template <class F, std::size_t K>
SquareMatrix<F, K> operator*(const SquareMatrix<F, K> factor1, const SquareMatrix<F, K> factor2) {
    SquareMatrix<F, K> result;
    for(size_t i = 0u; i < K; i++){
        for(size_t j = 0u; j < K; j++){
            result[j][i] = 0.0;
        }
    }
    for (size_t i = 0u; i < K; i++) {//Zeilen
        for (size_t j = 0u; j < K; j++) {//Spalten
            for(size_t k = 0u; k < K; k++) {//Stelle
                result[j][i] += factor1[k][i] * factor2[j][k];
            }
        }
    }
    return result;
}
