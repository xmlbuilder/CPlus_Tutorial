# TArray 기능 확장

## 소스 코드
```cpp
#pragma once

#include <math.h>
#include <assert.h>
#include <vector>
#include <functional>
#include <algorithm>

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


    std::vector<TArrRef<Type>> windows(int k) const {
        std::vector<TArrRef<Type>> result;
        if (k <= 0 || k > m_nSize) return result;
        for (int i = 0; i <= m_nSize - k; i++) {
            result.emplace_back(m_pData + i, k);
        }
        return result;
    }


    std::vector<TArrRef<Type>> chunks(int k) const {
        std::vector<TArrRef<Type>> result;
        if (k <= 0) return result;
        for (int i = 0; i < m_nSize; i += k) {
            int size = std::min(k, m_nSize - i);
            result.emplace_back(m_pData + i, size);
        }
        return result;
    }



    template <class ResultType>
    TArray<ResultType> map(std::function<ResultType(const Type&)> func) const {
        TArray<ResultType> result;
        result.setSize(m_nSize);
        for (int i = 0; i < m_nSize; i++) {
            result[i] = func(m_pData[i]);
        }
        return result;
    }

    TArray<Type> filter(std::function<bool(const Type&)> pred) const {
        std::vector<Type> tmp;
        for (int i = 0; i < m_nSize; i++) {
            if (pred(m_pData[i])) {
                tmp.push_back(m_pData[i]);
            }
        }
        TArray<Type> result;
        result.setSize((int)tmp.size());
        for (int i = 0; i < (int)tmp.size(); i++) {
            result[i] = tmp[i];
        }
        return result;
    }


    Type reduce(std::function<Type(const Type&, const Type&)> func, Type init) const {
        Type acc = init;
        for (int i = 0; i < m_nSize; i++) {
            acc = func(acc, m_pData[i]);
        }
        return acc;
    }


    void for_each(std::function<void(const Type&)> func) const {
        for (int i = 0; i < m_nSize; i++) {
            func(m_pData[i]);
        }
    }

    void for_each_mut(std::function<void(Type&)> func) {
        for (int i = 0; i < m_nSize; i++) {
            func(m_pData[i]);
        }
    }


    template <class OtherType, class ResultType>
    TArray<ResultType> zip(const TArray<OtherType>& other,
                           std::function<ResultType(const Type&, const OtherType&)> func) const {
        int n = std::min(m_nSize, other.getSize());
        TArray<ResultType> result;
        result.setSize(n);
        for (int i = 0; i < n; i++) {
            result[i] = func(m_pData[i], other[i]);
        }
        return result;
    }


    template <class ResultType>
    TArray<ResultType> flat_map(std::function<TArray<ResultType>(const Type&)> func) const {
        std::vector<ResultType> tmp;
        for (int i = 0; i < m_nSize; i++) {
            TArray<ResultType> inner = func(m_pData[i]);
            for (int j = 0; j < inner.getSize(); j++) {
                tmp.push_back(inner[j]);
            }
        }
        TArray<ResultType> result;
        result.setSize((int)tmp.size());
        for (int i = 0; i < (int)tmp.size(); i++) {
            result[i] = tmp[i];
        }
        return result;
    }


    TArray<Type> take_while(std::function<bool(const Type&)> pred) const {
        int count = 0;
        while (count < m_nSize && pred(m_pData[count])) {
            count++;
        }
        TArray<Type> result;
        result.setSize(count);
        for (int i = 0; i < count; i++) {
            result[i] = m_pData[i];
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

---

## 샘플 코드
```cpp
#include <iostream>
#include "TArray.hpp"
int main() {
    {
        TArray<int> arr;
        arr.setSize(5);
        for (int i = 0; i < arr.getSize(); i++) arr[i] = i + 1;

        auto squared = arr.map<int>([](const int& x) { return x * x; });

        auto evens = arr.filter([](const int& x) { return x % 2 == 0; });

        int sum = arr.reduce([](const int& a, const int& b) { return a + b; }, 0);

        std::cout << "arr results: " << std::endl;
        arr.for_each([](const int& x) { std::cout << x << " "; });
        std::cout << std::endl;
        std::cout << "squared: " << sum << std::endl;
        squared.for_each([](const int& x) { std::cout << x << " "; });
        std::cout << std::endl;
        std::cout << "evens: " << std::endl;
        evens.for_each([](const int& x) { std::cout << x << " "; });
        std::cout << std::endl;

        std::cout << "sum: " << sum << std::endl;
    }

    {
        TArray<int> arr1;
        arr1.setSize(10);
        for (int i = 0; i < arr1.getSize(); i++) arr1[i] = i + 1;


        std::cout << "windows results: " << std::endl;
        // windows(3)
        auto win = arr1.windows(3);
        for (auto& w : win) {
            for (int i = 0; i < w.getSize(); i++) {
                std::cout << w[i] << " ";
            }
            std::cout << "\n";
        }

        std::cout << "chunks results: " << std::endl;
        // chunks(4)
        auto ch = arr1.chunks(4);
        for (auto& c : ch) {
            for (int i = 0; i < c.getSize(); i++) {
                std::cout << c[i] << " ";
            }
            std::cout << "\n";
        }
    }

    {
        TArray<int> arr;
        arr.setSize(5);
        for (int i = 0; i < arr.getSize(); i++) arr[i] = i + 1;

        TArray<int> arr2;
        arr2.setSize(5);
        for (int i = 0; i < arr2.getSize(); i++) arr2[i] = (i+1)*10;

        auto zipped = arr.zip<int,int>(arr2, [](int a, int b){ return a+b; });

        std::cout << "zipped results: " << std::endl;
        zipped.for_each([](const int& x) { std::cout << x << " "; });
        std::cout << "\n";

        auto fm = arr.flat_map<int>([](int x){
            TArray<int> tmp;
            tmp.setSize(2);
            tmp[0] = x;
            tmp[1] = x*2;
            return tmp;
        });

        std::cout << "fm results: " << std::endl;
        fm.for_each([](const int& x) { std::cout << x << " "; });
        std::cout << "\n";

        std::cout << "tw results: " << std::endl;
        auto tw = arr.take_while([](int x){ return x < 4; });
        tw.for_each([](const int& x) { std::cout << x << " "; });
        std::cout << "\n";
    }
    return 0;
}
```
### 출력 결과
```
arr results:
1 2 3 4 5
squared: 15
1 4 9 16 25
evens:
2 4
sum: 15
windows results:
1 2 3
2 3 4
3 4 5
4 5 6
5 6 7
6 7 8
7 8 9
8 9 10
chunks results:
1 2 3 4
5 6 7 8
9 10
zipped results:
11 22 33 44 55
fm results:
1 2 2 4 3 6 4 8 5 10
tw results:
1 2 3
```
