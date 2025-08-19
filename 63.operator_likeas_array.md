# 🧠 C++에서 배열처럼 객체를 다루는 operator()와 operator[]
C++에서는 연산자 오버로딩을 통해 사용자 정의 클래스도 배열처럼 사용할 수 있습니다.  특히 **행렬(Matrix)**이나 2차원 데이터 구조를 다룰 때 operator()와 operator[]는 매우 유용하게 쓰입니다.

## 🔹 operator() — 다차원 인덱싱에 적합
```cpp
Type& operator()(int i, int j) {
    return m[i][j];
}

const Type& operator()(int i, int j) const {
    return m[i][j];
}
```

- operator()는 함수 호출 연산자이지만, 인덱스 접근용으로 오버로딩하면 mat(i, j)처럼 사용할 수 있습니다.
- 위 예제에서는 m이 2차원 배열 또는 std::vector<std::vector<Type>>일 가능성이 높습니다.
- const 버전은 읽기 전용 접근을 허용합니다.
## ✅ 사용 예시
```cpp
ON_Matrix mat(5, 5);

for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
        mat(i, j) = i + j;  // 값 설정
    }
}

for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
        std::cout << mat(i, j) << ", ";  // 값 출력
    }
    std::cout << std::endl;
}
```

- mat(i, j)는 마치 2차원 배열처럼 동작합니다.
- 내부적으로는 m[i][j]에 접근하는 방식입니다.

### 🔸 operator[] — 1차원 인덱싱에 적합
```cpp
Type& operator[](int i) {
    return m[i];
}

const Type& operator[](int i) const {
    return m[i];
}
```

- operator[]는 단일 인덱스 접근에 사용됩니다.
- 1차원 배열이나 벡터를 다룰 때 적합합니다.
- 2차원 배열을 다루려면 mat[i][j]처럼 두 번 접근해야 하므로, operator()보다 덜 직관적일 수 있습니다.

### 🎯 비교: operator() vs operator[]
| 연산자 | 사용 형태 | 인덱스 수 | 직관성 | 추천 용도 |
|-------|----------|----------|-------|----------| 
| operator() | mat(i, j) | 2개 이상 | 높음 | 행렬, 2D 구조체 | 
| operator[] | mat[i][j] | 1개씩 접근 | 낮음 | 벡터, 1D 배열 | 



## 🧵 요약
- operator()는 다차원 인덱싱을 깔끔하게 처리할 수 있어 행렬 클래스에 자주 사용됩니다.
- operator[]는 단일 인덱스 접근에 적합하며, 내부적으로 또 다른 operator[]를 반환해야 2차원처럼 사용할 수 있습니다.
- 두 연산자 모두 객체를 배열처럼 사용할 수 있게 해주는 핵심 도구입니다.

---

## 🧮 OpenNURBS의 ON_Matrix 클래스 설명
ON_Matrix는 OpenNURBS에서 제공하는 일반적인 행렬 클래스로, 수치 계산이나 기하학적 변환에 사용됩니다. 이 클래스는 내부적으로 2차원 배열처럼 동작하며, operator()를 통해 배열처럼 접근할 수 있게 변경 할 수 있습니다.

### 🔹 내부 구조
class ON_Matrix {
public:
  int m_row_count;
  int m_col_count;
  double* m; // 1D 배열로 저장된 행렬 데이터

  // ...
  double& operator()(int i, int j);
  const double& operator()(int i, int j) const;
};


- 행렬 데이터는 double* m으로 1차원 배열로 저장됩니다.
- 실제로는 m[i * m_col_count + j] 방식으로 2차원처럼 접근합니다.
- operator()는 이 인덱싱을 감싸서 사용자가 2차원 배열처럼 접근할 수 있도록 도와줍니다.

### 🔍 operator() 구현 예시
```cpp
double& ON_Matrix::operator()(int i, int j) {
  return m[i * m_col_count + j];
}

const double& ON_Matrix::operator()(int i, int j) const {
  return m[i * m_col_count + j];
}
```

- i는 행 인덱스, j는 열 인덱스입니다.
- 내부적으로는 1차원 배열이지만, 외부에서는 mat(i, j)처럼 직관적으로 사용할 수 있습니다.
### ✅ 사용 예시
```cpp
ON_Matrix mat(5, 5); // 5x5 행렬 생성

for (int i = 0; i < 5; i++) {
  for (int j = 0; j < 5; j++) {
    mat(i, j) = i + j; // 값 설정
  }
}

for (int i = 0; i < 5; i++) {
  for (int j = 0; j < 5; j++) {
    std::cout << mat(i, j) << ", "; // 값 출력
  }
  std::cout << std::endl;
}
```

- mat(i, j)는 내부적으로 m[i * 5 + j]에 접근합니다.
- 사용자 입장에서는 2차원 배열처럼 직관적으로 사용할 수 있습니다.

### 🎯 요약
| 항목 | 설명 |
|------|-----| 
| 저장 방식 | 1차원 배열 (double* m) | 
| 접근 방식 | operator()(i, j) 오버로딩 | 
| 장점 | 2차원 배열처럼 직관적 사용 가능 | 
| 활용 분야 | 기하학적 계산, 변환 행렬, 수치 해석 등 | 

---




