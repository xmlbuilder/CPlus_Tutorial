## 📑 Chunks와 Windows 기능 필요성 문서
### 1. 개요
- Chunks: 배열을 일정한 크기 단위로 나누어 처리하는 기능
- Windows: 배열에서 연속된 부분 구간(슬라이딩 윈도우)을 추출하는 기능
- 두 기능 모두 원본 데이터를 복사하지 않고 참조 기반으로 부분 배열을 제공하여 효율적인 데이터 처리와 알고리즘 구현을 가능하게 한다.

### 2. Chunks가 필요한 이유
- 대용량 데이터 처리: 큰 배열을 일정 크기로 나누어 병렬 처리하거나 배치(batch) 단위로 다루기 용이
- 알고리즘 단순화: 예를 들어, 1000개의 데이터를 100개 단위로 나누어 반복문을 단순화
- 메모리 효율성: 복사 없이 참조만 제공하므로 추가 메모리 부담이 적음
- 응용 사례
  - 이미지/신호 처리에서 블록 단위 연산
  - 데이터 스트리밍에서 패킷 단위 처리
  - 통계 분석에서 그룹별 집계

### 3. Windows가 필요한 이유
- 패턴 탐색: 연속된 부분 배열을 쉽게 얻을 수 있어 시계열 데이터 분석에 유용
- 슬라이딩 연산: 이동 평균, 이동 합계, 이상치 탐지 등에서 필수
- 알고리즘 최적화: 문자열 검색(KMP, Rabin-Karp), 신호 처리(FIR 필터) 등에서 윈도우 기반 접근이 자연스러움
- 응용 사례
  - 금융 데이터에서 이동 평균 계산
  - 머신러닝에서 시퀀스 특징 추출
  - 문자열/텍스트 처리에서 n-gram 생성

### 4. 비교 요약
| 기능       | 설명                          | 주요 활용 분야                  |
|------------|-------------------------------|---------------------------------|
| Chunks     | 배열을 고정 크기 블록으로 분할 | 배치 처리, 병렬 연산, 블록 단위 분석 |
| Windows    | 배열에서 연속된 부분 구간 추출 | 이동 평균, 패턴 탐색, 시계열 분석   |


### 5. 결론
- Chunks는 데이터를 나누어 처리하는 데 강점이 있고,
- Windows는 데이터를 연속적으로 탐색하는 데 강점이 있다.
- 두 기능을 지원하면 배열 기반 연산의 표현력이 크게 향상되며,  
  Rust처럼 안전성과 효율성을 중시하는 언어에서 널리 쓰이는 이유도 여기에 있다.

---

```cpp
#pragma once

#include "TString.hpp"
#include <math.h>
#include <assert.h>
#include <vector>

#ifndef MIN
#define MIN(a,b) ( ((a)>(b)) ? (b) : (a) )
#endif

template<class Type>
class TArrRef
{
public:
    TArrRef() : m_pData(0), m_nSize(0) {}
    TArrRef(const Type* pData, int nSize) : m_pData(pData), m_nSize(nSize) {}
    ~TArrRef() {}	// not delete memory, it's just for reference

    void  setData(const Type* pData) { m_pData = pData; }
    void  setSize(int nSize)  { m_nSize = nSize; }
    void  setCount(int nCount) { m_nSize = nCount; }

    const Type* getData() const { return m_pData; }
    int  getSize() const { return m_nSize; }
    int  getCount() const { return m_nSize; }

    const Type& operator[](int nIndex)
    {
        assert( nIndex >= 0 && nIndex < m_nSize);
        return m_pData[nIndex];
    }


protected:
    const Type*	m_pData;
    int 	m_nSize;
};
```
```cpp
template <class Type>
class TArray	
{
public:
    TArray() : m_pData(nullptr), m_nSize(0) {}
    TArray(const TArray<Type>& rhs) : m_pData(nullptr), m_nSize(0)
	{
        if (rhs.m_nSize)
		{
            m_pData = new Type[rhs.m_nSize];
            memcpy( m_pData, rhs.m_pData, rhs.m_nSize * sizeof(Type) );
            m_nSize = rhs.m_nSize;
		}
	}

    ~TArray() { removeAll(); }

	TArray<Type>& operator=(const TArray<Type>& rhs)
	{
        if( 0 == rhs.m_nSize ){	removeAll(); }
		else
		{
            Type* pTmp = new Type[rhs.m_nSize];
            memcpy(pTmp, rhs.m_pData, rhs.m_nSize*sizeof(Type));
			delete [] m_pData;
			m_pData = pTmp;
            m_nSize = rhs.m_nSize;
		}
		return *this;
	}

	// Operations
    int getSize() const { return m_nSize; }
    int getCount() const { return m_nSize; }

    const Type& operator[](int nIndex) const
	{
        assert( nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex];
	}

    Type& operator[](int nIndex)
	{
        assert( nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex];
	}

    void setSize(const int lSize)
	{
        if( lSize == m_nSize ) return;
		if( lSize <= 0 )
		{
            removeAll();
			return;
		}
		Type* pTmp = new Type[lSize];
		delete [] m_pData;
		m_pData = pTmp;
        m_nSize = lSize;
	}

    void removeAll()
	{
		if(m_pData){
            delete [] m_pData;
			m_pData = NULL;
		}
        m_nSize = 0;
	}

    bool isEmpty() const
	{
        return (m_nSize==0);
	}

    void resize(const int lSize)
	{
		if( lSize <= 0 )
		{
            removeAll();
			return;
		}
        if(m_nSize >= lSize)
		{
            m_nSize = lSize;
			return;
		}
		Type* pTmp = new Type[lSize];
        memcpy(pTmp, m_pData, m_nSize*sizeof(Type));
		delete [] m_pData;
		m_pData = pTmp;
        m_nSize = lSize;
	}
    Type* getData() const { return m_pData; }


    void calcMaxMinValue(Type& dMax, Type& dMin) const
    {
        int lCnt = m_nSize;
        if(lCnt == 0) return;
        dMin = m_pData[0];
        dMax = m_pData[0];
        for(int i=1;i<lCnt;i++)
        {
            if(m_pData[i] > dMax)
            {
                dMax = m_pData[i];
            }
            if(m_pData[i] < dMin)
            {
                dMin = m_pData[i];
            }
        }
    }


    void calcAbsMaxMinValue(Type& dMax, Type& dMin) const
    {
        int lCnt = m_nSize;
        if(lCnt == 0) return;
        dMin = fabs(m_pData[0]);
        dMax = fabs(m_pData[0]);
        for(int i=1;i<lCnt;i++)
        {
            Type data = fabs(m_pData[i]);
            if(data > dMax)
            {
                dMax = data;
            }
            if(data < dMin)
            {
                dMin = data;
            }
        }
    }

    void calcAbsMaxMinValueRange(Type& dMax, Type& dMin, int startOffset) const
    {
        int lCnt = m_nSize;
        if(lCnt == 0) return;
        dMin = fabs(m_pData[0+startOffset]);
        dMax = fabs(m_pData[0+startOffset]);
        for(int i=1+startOffset;i<lCnt-startOffset;i++)
        {
            Type data = fabs(m_pData[i]);
            if(data > dMax)
            {
                dMax = data;
            }
            if(data < dMin)
            {
                dMin = data;
            }
        }
    }

    // windows: 슬라이딩 윈도우
    std::vector<TArrRef<Type>> windows(int k) const {
        std::vector<TArrRef<Type>> result;
        if (k <= 0 || k > m_nSize) return result;
        for (int i = 0; i <= m_nSize - k; i++) {
            result.emplace_back(m_pData + i, k);
        }
        return result;
    }

    // chunks: 고정 크기 청크
    std::vector<TArrRef<Type>> chunks(int k) const {
        std::vector<TArrRef<Type>> result;
        if (k <= 0) return result;
        for (int i = 0; i < m_nSize; i += k) {
            int size = std::min(k, m_nSize - i);
            result.emplace_back(m_pData + i, size);
        }
        return result;
    }


protected:
    Type*	 m_pData;
    int      m_nSize;
};

struct TArrPart 
{
    int 	idxStart;
    int 	nSize;

    TArrPart(int _idxStart = 0, int _nSize = 0) : idxStart(_idxStart), nSize(_nSize) {}
    bool isEmpty() { return (nSize==0); }

    int start() { return idxStart; }
    int end() { return (idxStart + nSize); }

    void adjust(int nParentArraySize)
	{
		if( idxStart < nParentArraySize )
		{
			nSize = MIN( idxStart + nSize, nParentArraySize ) - idxStart;
			if( nSize < 0 ) nSize = 0;
		}
		else
		{
			idxStart = 0;
			nSize = 0;
		}
	}

	void inc() { nSize++; }

};

typedef TArray<int8_t>              TArrayB;
typedef TArray<unsigned char>       TArrayub;
typedef TArray<unsigned int>        TArrayui;
typedef TArray<unsigned short>      TArrayus;

typedef TArray<long>                TArrayl;
typedef TArray<int>                 TArrayi;
typedef TArray<float>               TArrayf;
typedef TArray<double>              TArrayd;
```
## 샘플 코드
```cpp
TArray<int> arr;
arr.setSize(10);
for (int i = 0; i < arr.getSize(); i++) arr[i] = i + 1;

// windows(3)
auto win = arr.windows(3);
for (auto& w : win) {
    for (int i = 0; i < w.getSize(); i++) {
        std::cout << w[i] << " ";
    }
    std::cout << "\n";
}
// 출력: 1 2 3 / 2 3 4 / ... / 8 9 10

// chunks(4)
auto ch = arr.chunks(4);
for (auto& c : ch) {
    for (int i = 0; i < c.getSize(); i++) {
        std::cout << c[i] << " ";
    }
    std::cout << "\n";
}
```
```
// 출력: 1 2 3 4 / 5 6 7 8 / 9 10
```
