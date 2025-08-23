# 📘 문자열 유틸리티 개선 및 문서

## 1. using namespace std; 지양하기
- 전역 네임스페이스을 가급적 쓰지 말것
- std::를 명시적으로 사용하는 것이 안전합니다.

## 2. 공백 제거 함수 (trim, ltrim, rtrim)
### ✅ 현재 구현

```cpp
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);
std::string ltrim_copy(std::string s);
std::string rtrim_copy(std::string s);
std::string trim_copy(std::string s);
```

```cpp
void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), 
    [](unsigned char c) {
        return !std::isspace(c);
    }));
}

void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](int c) {return !std::isspace(c);} ).base(), s.end());
}

void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

std::string ltrim_copy(std::string s) {

    ltrim(s);
    return s;
}

std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

std::string trim_copy(std::string s) {

    trim(s);
    return s;
}
```

## 3. 문자열 대소문자 변환 (ToUpper, ToLower, strUpr, strLwr)
```cpp
int strUpr(char *str)
{
    int i = 0;
    int len = 0;
    len = (int)strlen(str);
    for(i=0;i<len;i++){

        *(str+i)=std::toupper(*(str+i));
    }
    return i;
}

int strLwr(char *str)
{
    int i=0;
    int len=0;
    len = (int)strlen(str);
    for(i=0;i<len;i++){
        *(str+i)=std::tolower(*(str+i));
    }
    return i;
}

std::string ToUpper(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}


std::string ToLower(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}
```

### 🔧 개선 포인트
- std::string_view를 활용하면 복사 비용 없이 처리 가능 (C++17 이상)



## 4. 문자열 토큰 분리 (tokenParam, tokenpParamWithEmpty)

### ✅ 현재 구현
- std::regex와 sregex_token_iterator를 사용하여 분리
```cpp
bool tokenParam(const std::string& strParam, 
    std::vector<std::string>& aParam)
{
    aParam.clear();
    std::string str = strParam;
    regex reg("[,\\s+]");
    sregex_token_iterator iter(str.begin(), str.end(), reg, -1);
    sregex_token_iterator end;
    vector<string> vec(iter, end);
    for (const auto& a : vec)
    {
        if(!a.empty()){
            aParam.push_back(a);
        }
    }
    return true;
}

bool tokenParam(const std::string& strParam, 
    const std::string& strTok, 
    std::vector<std::string>& aParam)
{
    aParam.clear();
    std::string str = strParam;
    regex reg(strTok);
    sregex_token_iterator iter(str.begin(), str.end(), reg, -1);
    sregex_token_iterator end;
    vector<string> vec(iter, end);
    for (const auto& a : vec)
    {
        if(!a.empty()){
            aParam.push_back(a);
        }
    }
    return true;
}

bool tokensParamWithEmpty(const std::string& strParam, 
    std::string strTok, 
    std::vector<std::string>& aParam)
{
    aParam.clear();
    std::string str = strParam;
    regex reg(strTok);
    sregex_token_iterator iter(str.begin(), str.end(), reg, -1);
    sregex_token_iterator end;
    vector<string> vec(iter, end);
    for (const auto& a : vec)
    {
        aParam.push_back(a);
    }
    return true;
}
```

### 🔧 개선 포인트
- std::string_view를 사용하면 성능 향상 가능


## 5. 문자열 → 실수 변환 (strToFloat)
### ✅ 현재 구현

```cpp
double strToFloat(const std::string& str, bool& error) {
    error = false;
    char* endPtr = nullptr;
    double result = std::strtod(str.c_str(), &endPtr);
    if (endPtr == str.c_str()) error = true;
    return result;
}
```
- 속도 때문에 사용자가 만들어서 사용 할 수도 있다.
```cpp
double StringToDouble(const Type *szBuf, bool &error)
{
	error = false;
	register double number;
	register int exponent;
	register int negative;
	register Type *p = (Type *) szBuf;
	register double p10;
	register int n;
	register int num_digits;
	register int num_decimals;

	while (isspace(*p)) p++;
	negative = 0;
	switch (*p) 
	{             
	case '-': negative = 1; 
	case '+': p++;
	}

	number = 0.;
	exponent = 0;
	num_digits = 0;
	num_decimals = 0;
	while (isdigit(*p))
	{
		number = number * 10. + (*p - '0');
		p++;
		num_digits++;
	}

	if (*p == '.')
	{
		p++;
		while (isdigit(*p))
		{
			number = number * 10. + (*p - '0');
			p++;
			num_digits++;
			num_decimals++;
		}
		exponent -= num_decimals;
	}

	if (num_digits == 0)
	{
		error = true;
		return 0.0;
	}
	if (negative) number = -number;

	if (*p == 'e' || *p == 'E' || *p == 'D' || *p == 'd') 
	{
		negative = 0;
		switch(*++p) 
		{   
		case '-': negative = 1;
		case '+': p++;
		}
		n = 0;
		while (isdigit(*p)) 
		{   
			n = n * 10 + (*p - '0');
			p++;
		}
		if (negative) 
			exponent -= n;
		else
			exponent += n;
	}
	else if(*p == '-' || *p == '+')
	{
		negative = 0;
		switch(*p) 
		{   
		case '-': negative = 1;
		case '+': p++;
		}
		n = 0;
		while (isdigit(*p))
		{   
			n = n * 10 + (*p - '0');
			p++;
		}

		if (negative) 
			exponent -= n;
		else
			exponent += n;
	}

	if (exponent < DBL_MIN_EXP  || exponent > DBL_MAX_EXP)
	{
		errno = true;
		return HUGE_VAL;
	}

	p10 = 10.;
	n = exponent;
	if (n < 0) n = -n;
	while (n) 
	{
		if (n & 1) 
		{
			if (exponent < 0)
				number /= p10;
			else
				number *= p10;
		}
		n >>= 1;
		p10 *= p10;
	}
	if (number == HUGE_VAL) error = true;
	return number;
}

```




