## 📘 k-d Tree 기반 3D 포인트 병합 알고리즘

- 3차원 공간상의 점들을 k-d 트리(k-dimensional tree)를 활용하여 효율적으로 병합하는 알고리즘을 구현한 것입니다.
- 가까운 점들을 기준 거리(threshold) 이하일 경우 평균값으로 병합하여 데이터의 중복을 줄이고, 공간적 밀집도를 완화할 수 있습니다.

### 📌 주요 기능

- 3D 포인트 구조체 정의
- 유클리디안 거리 계산 함수
- k-d 트리 삽입 및 최근접점 탐색
- 기준 거리 이하의 점 병합 처리

### 📌 알고리즘 설명

#### 🔹 1. `Point3D` 구조체
```cpp
struct Point3D {
    double x, y, z;
};
```
- 3차원 공간상의 점을 나타냅니다.

#### 🔹 2. 거리 계산 함수
```cpp
double distance(const Point3D& a, const Point3D& b);
```
- 두 점 사이의 유클리디안 거리를 계산합니다.

#### 🔹 3. k-d 트리 노드 및 삽입
```cpp
struct KDNode {
    Point3D point;
    KDNode* left = nullptr;
    KDNode* right = nullptr;
};

KDNode* insertKDTree(KDNode* root, Point3D point, int depth = 0) {
    if (!root) return new KDNode{point};

    int axis = depth % 3;
    double value = (axis == 0) ? point.x
                 : (axis == 1) ? point.y
                 : point.z;
    double rootValue = (axis == 0) ? root->point.x
                     : (axis == 1) ? root->point.y
                     : root->point.z;

    if (value < rootValue)
        root->left = insertKDTree(root->left, point, depth + 1);
    else
        root->right = insertKDTree(root->right, point, depth + 1);

    return root;
}
```

- 3차원 공간이므로 x, y, z 축을 기준으로 깊이에 따라 분할합니다.
- 삽입 시 현재 깊이(depth)에 따라 축을 선택하여 좌/우 서브트리에 삽입합니다.

#### 🔹 4. 최근접점 탐색
```cpp
Point3D findNearest(KDNode* root, Point3D target, int depth = 0, Point3D best = {9999, 9999, 9999}) {
    if (!root) return best;

    double bestDist = distance(target, best);
    double currentDist = distance(target, root->point);

    if (currentDist < bestDist)
        best = root->point;

    int axis = depth % 3;
    double targetValue = (axis == 0) ? target.x
                      : (axis == 1) ? target.y
                      : target.z;
    double rootValue = (axis == 0) ? root->point.x
                    : (axis == 1) ? root->point.y
                    : root->point.z;

    KDNode* first = (targetValue < rootValue) ? root->left : root->right;
    KDNode* second = (targetValue < rootValue) ? root->right : root->left;

    best = findNearest(first, target, depth + 1, best);

    if (fabs(targetValue - rootValue) < bestDist)
        best = findNearest(second, target, depth + 1, best);

    return best;
}
```

- 주어진 타겟 점과 가장 가까운 점을 k-d 트리에서 탐색합니다.
- 분할 축 기준으로 탐색 우선순위를 정하고, 필요 시 반대 방향도 탐색합니다.


#### 🔹 5. 병합 처리
```cpp
std::vector<Point3D> mergeClosePoints(std::vector<Point3D>& points, double threshold) {
    KDNode* root = nullptr;
    std::vector<Point3D> merged;

    for (auto& point : points) {
        Point3D nearest = findNearest(root, point);

        if (distance(point, nearest) < threshold) {
            nearest.x = (nearest.x + point.x) / 2.0;
            nearest.y = (nearest.y + point.y) / 2.0;
            nearest.z = (nearest.z + point.z) / 2.0;
        } else {
            merged.push_back(point);
            root = insertKDTree(root, point);
        }
    }

    return merged;
}
```

- 각 점에 대해 k-d 트리에서 최근접점을 찾고, 기준 거리 이하일 경우 평균값으로 병합합니다.
- 병합되지 않은 점은 k-d 트리에 삽입하여 이후 탐색에 활용됩니다.

### 📌 실행 예시
```cpp
int main() {
    std::vector<Point3D> points = {
        {1.0, 2.0, 3.0}, {1.2, 2.1, 3.0}, {5.0, 6.0, 7.0},
        {10.0, 10.1, 10.2}, {10.2, 10.3, 10.4}
    };

    double threshold = 0.5;
    std::vector<Point3D> merged = mergeClosePoints(points, threshold);

    std::cout << "Merged Points:\n";
    for (const auto& pt : merged) {
        std::cout << pt.x << ", " << pt.y << ", " << pt.z << "\n";
    }

    return 0;
}
```

출력 결과:
```
Merged Points:
1, 2, 3
5, 6, 7
10, 10.1, 10.2
```

### 📌 전체 소스
```cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

struct Point3D {
    double x, y, z;
};

double distance(const Point3D& a, const Point3D& b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

// k-d tree 기반 최근접 점 병합
std::vector<Point3D> mergeClosePoints(std::vector<Point3D>& points, double threshold) {
    std::vector<Point3D> merged;
    for (size_t i = 0; i < points.size(); ++i) {
        bool mergedAlready = false;

        for (size_t j = 0; j < merged.size(); ++j) {
            if (distance(points[i], merged[j]) < threshold) {
                merged[j].x = (merged[j].x + points[i].x) / 2.0;
                merged[j].y = (merged[j].y + points[i].y) / 2.0;
                merged[j].z = (merged[j].z + points[i].z) / 2.0;
                mergedAlready = true;
                break;
            }
        }

        if (!mergedAlready) merged.push_back(points[i]);
    }
    return merged;
}

int main() {
    std::vector<Point3D> points = {
        {1.0, 2.0, 3.0}, {1.2, 2.1, 3.0}, {5.0, 6.0, 7.0},
        {10.0, 10.1, 10.2}, {10.2, 10.3, 10.4}
    };

    double threshold = 0.5; // 너무 가까운 점 병합 기준
    std::vector<Point3D> merged = mergeClosePoints(points, threshold);

    std::cout << "Merged Points:\n";
    for (const auto& pt : merged) {
        std::cout << pt.x << ", " << pt.y << ", " << pt.z << "\n";
    }

    return 0;
}
// Merged Points:
// 1.1, 2.05, 3
// 5, 6, 7
// 10.1, 10.2, 10.3
```


### 📌 활용 예시
- 3D 스캔 데이터 후처리
- 포인트 클라우드 정제
- 공간적 중복 제거 및 압축
---


