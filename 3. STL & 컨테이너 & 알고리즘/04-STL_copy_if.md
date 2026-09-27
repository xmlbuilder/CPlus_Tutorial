## 📘 stl::copy_if 
- 응용 - Point3D Distance Filter 에 적용해 보기

### 📌 std::copy_if란?

- `std::copy_if`는 C++ 표준 라이브러리 `<algorithm>`에 포함된 알고리즘으로,  
**주어진 범위에서 조건(Predicate)을 만족하는 원소만 다른 컨테이너로 복사** 할 때 사용합니다.

#### 형태
```cpp
template<class InputIt, class OutputIt, class UnaryPredicate>
OutputIt copy_if(InputIt first, InputIt last, OutputIt d_first, UnaryPredicate pred);
```
- **매개변수**
    - `first`, `last` : 입력 범위
    - `d_first` : 복사할 대상 컨테이너의 시작 반복자
    - `pred` : 원소를 복사할지 여부를 결정하는 조건 함수(또는 람다)
- **반환값** : 마지막으로 복사한 원소 다음 위치의 출력 반복자
- **특징**:
    - 원소의 **순서를 유지** 하면서 조건에 맞는 것만 복사
    - 대상 컨테이너는 미리 크기를 확보해 두거나, `std::back_inserter`를 사용

#### ✅ 간단 예시:
```cpp
#include <vector>
#include <algorithm>
#include <iostream>

int main() {
    std::vector<int> src = {1, 2, 3, 4, 5};
    std::vector<int> dst;

    std::copy_if(src.begin(), src.end(), std::back_inserter(dst),
                 [](int x){ return x % 2 == 0; });

    for (int v : dst) std::cout << v << " "; // 2 4
}
```


### 📌 이 예제에서의 활용

- 이 프로젝트에서는 `std::copy_if`를 사용해, 기준점(`center`)에서 일정 거리(`threshold`) 안에 있는 3D 포인트만 걸러냅니다.

#### ✨ 핵심 포인트
- `Point3D::DistanceTo` : 유클리드 거리 계산
- `std::copy_if` + `std::back_inserter` : 조건을 만족하는 원소만 `filtered`에 복사
- 람다 캡처 `[&]` : `center`와 `threshold`를 람다에서 사용
- 시간복잡도: O(n)

#### 🧾 코드

```cpp
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

struct Point3D {
    double x, y, z;

    double DistanceTo(const Point3D& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        double dz = z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

int main() {
    std::vector<Point3D> points = {
        {1, 2, 3}, {2, 3, 4}, {10, 10, 10}, {3, 4, 5}
    };

    Point3D center = {2, 3, 4};
    double threshold = 5.0;

    std::vector<Point3D> filtered;

    std::copy_if(points.begin(), points.end(), std::back_inserter(filtered),
        [&](const Point3D& p) {
            return p.DistanceTo(center) < threshold;
        });

    for (const auto& p : filtered) {
        std::cout << "Filtered Point: (" << p.x << ", " << p.y << ", " << p.z << ")\n";
    }

    return 0;
}
```

### ▶️ 빌드 & 실행

#### g++
```bash
g++ -std=c++17 -O2 main.cpp -o filter_points
./filter_points
```

#### (선택) CMake
- `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.12)
project(Point3DFilter CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(filter_points main.cpp)
```
- 빌드:
```bash
cmake -S . -B build
cmake --build build --config Release
./build/filter_points
```

### 🧪 예상 출력
```
Filtered Point: (1, 2, 3)
Filtered Point: (2, 3, 4)
Filtered Point: (3, 4, 5)
```

### 🛠️ 팁 (성능/안정성)
- **sqrt 없이 제곱거리 비교** 로 성능 미세 개선:
  ```cpp
  auto sqdist = [&](const Point3D& a, const Point3D& b){
      double dx=a.x-b.x, dy=a.y-b.y, dz=a.z-b.z;
      return dx*dx + dy*dy + dz*dz;
  };
  double thr2 = threshold * threshold;
  std::copy_if(points.begin(), points.end(), std::back_inserter(filtered),
               [&](const Point3D& p){ return sqdist(p, center) < thr2; });
  ```
- 큰 데이터를 미리 예상한다면 `filtered.reserve(points.size());`로 재할당 줄이기
- 임계값 경계 포함 여부는 기획에 맞춰 `<` vs `<=` 선택
- NaN 입력 가능성이 있으면 `std::isfinite` 검사 추가 고려

---


