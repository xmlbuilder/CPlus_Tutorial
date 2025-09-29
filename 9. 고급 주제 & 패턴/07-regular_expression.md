# 🧠 C++ Regular Expression (<regex>) 샘플 정리
## 📦 기본 개념
C++11부터 <regex> 헤더를 통해 **정규 표현식(Regular Expression)**을 사용할 수 있습니다.
### 주요 클래스:
- std::regex: 정규식 객체
- std::regex_match: 전체 문자열이 정규식과 일치하는지 검사
- std::regex_search: 부분 문자열이 정규식과 일치하는지 검사
- std::smatch: 매칭 결과를 저장하는 객체 (string match)

## 🔍 예제 코드 설명
```cpp
std::string fnames[] = {"foo.txt", "bar.txt", "a0.txt", "AAA.txt"};
std::regex txt_regex("[a-z]+\\.txt");
```

- [a-z]+ : 소문자 하나 이상
- \\.txt : .txt 확장자
- 전체적으로 "소문자들.txt" 형식만 매칭
### ✅ std::regex_match
```cpp
std::regex_match(fname, txt_regex)
```

- 문자열 전체가 정규식과 완전히 일치해야 true
- 출력 결과:
```
foo.txt : 1
bar.txt : 1
a0.txt  : 0
AAA.txt : 0
```


### 🎯 그룹 추출 예제
```cpp
std::regex base_regex("([a-z]+)\\.(txt)");
std::smatch base_match;
```

- ([a-z]+) : 첫 번째 그룹 → 파일명
- (txt)    : 두 번째 그룹 → 확장자
- base_match[0] : 전체 매칭된 문자열
- base_match[1] : 첫 번째 그룹
- base_match[2] : 두 번째 그룹

#### ✅ 출력 결과
```
foo.txt : foo
extension txt
bar.txt : bar
extension txt
```


- a0.txt와 AAA.txt는 매칭되지 않음 (소문자만 허용)

## 📘 주요 정규식 문법 요약표
| 패턴 | 의미 | 예시 매칭 값 | 
|------|-----|-------------|
| . | 아무 문자 하나 | a, 1, @ | 
| [abc] | a, b, 또는 c 중 하나 | a, b | 
| [^abc] | a, b, c 제외한 문자 | d, x | 
| [a-z] | 소문자 a~z | g, m | 
| [A-Z] | 대문자 A~Z | F, Z | 
| [0-9] | 숫자 0~9 | 3, 7 | 
| + | 앞 패턴 1회 이상 반복 | abc, a | 
| * | 앞 패턴 0회 이상 반복 | "", aaa | 
| ? | 앞 패턴 0 또는 1회 | a, "" | 
| () | 그룹 지정 | ([a-z]+) | 
| \\. | 마침표 문자 | . | 
| ^ | 문자열 시작 | ^abc → abc | 
| $ | 문자열 끝 | abc$ → abc | 


## 🧪 실전 팁
- std::regex_match → 전체 일치 검사
- std::regex_search → 부분 일치 검사
- std::smatch → 그룹 추출 시 필수
- std::regex_constants::icase → 대소문자 무시 옵션

## ✅ 보강 아이디어
### 🔤 대소문자 구분 없이 매칭
std::regex txt_regex("[a-z]+\\.txt", std::regex_constants::icase);


- "AAA.txt"도 매칭됨
### 🔍 확장자만 추출
std::regex ext_regex("\\.([a-z]+)$");

- .txt, .cpp, .jpg 등 확장자 추출 가능

---

## 🔧 1. std::regex_replace 예제 — 문자열 치환
```cpp
#include <iostream>
#include <regex>

void regex_replace_example() {
    std::string text = "The quick brown fox jumps over the lazy dog.";
    std::regex vowel_regex("[aeiou]");

    // 모든 모음을 '*'로 치환
    std::string replaced = std::regex_replace(text, vowel_regex, "*");

    std::cout << "Original: " << text << std::endl;
    std::cout << "Replaced: " << replaced << std::endl;
}
```

## 🧾 출력 결과
```
Original: The quick brown fox jumps over the lazy dog.
Replaced: Th* q**ck br*wn f*x j*mps *v*r th* l*zy d*g.
```


## 🔍 2. std::regex_search 예제 — 부분 문자열 검색
```cpp
#include <iostream>
#include <regex>

void regex_search_example() {
    std::string text = "Contact: user123@example.com";
    std::regex email_regex("[\\w.-]+@[\\w.-]+");

    std::smatch match;
    if (std::regex_search(text, match, email_regex)) {
        std::cout << "Found email: " << match[0] << std::endl;
    }
}
```

### 🧾 출력 결과
```
Found email: user123@example.com
```

- regex_search는 문자열 전체가 아니라 일부만 매칭해도 성공합니다.
- match[0]은 전체 매칭된 문자열

## 🗂️ 3. 파일 필터링 시스템 — .txt 파일만 추출
```cpp
#include <iostream>
#include <vector>
#include <regex>

void file_filter_example() {
    std::vector<std::string> files = {
        "report.docx", "data.csv", "notes.txt", "image.png", "readme.txt"
    };

    std::regex txt_filter(".*\\.txt$");

    std::cout << "Filtered .txt files:\n";
    for (const auto& file : files) {
        if (std::regex_match(file, txt_filter)) {
            std::cout << " - " << file << std::endl;
        }
    }
}
```

## 🧾 출력 결과
```
Filtered .txt files:
 - notes.txt
 - readme.txt
```

- .*\\.txt$ : 아무 문자열 + .txt로 끝나는 파일만 매칭
- 실전에서는 파일 시스템과 연동해 std::filesystem과 함께 사용하면 더 강력해져요

## 🧠 확장 아이디어
- ✅ std::filesystem과 정규식 조합 → 디렉토리 내 파일 필터링
- ✅ 로그 분석기 만들기 → regex_search로 특정 패턴 추출
- ✅ 사용자 입력 검증 → 이메일, 전화번호, URL 등

## 실전에 응용한 예
ISO 코드로 부터 특정한 이름을 가진  코드 이름으로 정보를 얻는 방법에 응용.

```cpp
#include <iostream>
#include <regex>
using namespace std;

int main() {

    std::string strFind = "3HEAD000000ACX";

    std::string sub1 = strFind.substr(0, 5);
    std::string sub2 = strFind.substr(strFind.size()-3, 3);

    std::string chnNames[] = {"03HEADTP00THACXA", "01HEAD0000H3ACYA", "01HEAD0000HMACZA", "01HEAD0000THACRA"};

    int cntItem = sizeof(chnNames) / sizeof(chnNames[0]);
    std::string strFindReg = "([0-6]" + sub1 + "\\w{6}" + sub2 + "\\w)";
    //std::regex txt_regex("([0-6]1HEAD\\w{6}ACY\\w)");

    std::regex txt_regex(strFindReg);
    std::smatch base_match;

    for (int i=0; i < cntItem; i++) {

        //std::cout << chnNames[i] << std::endl;
        if(std::regex_match(chnNames[i], base_match, txt_regex)) {
            std::cout << base_match.size() << std::endl;
            std::string base = base_match[1].str();
            std::cout << base << std::endl;
        }
    }
}



int ChannelManagerBase::findChannelArrayFromNamesRegex(std::vector<std::string>& vecName, std::vector<Channel*>& vecChannel)
{
    vecChannel.clear();
    for(auto & name : vecName)
    {
        std::string subName1 = name.substr(0, 5);
        std::string subName2 = name.substr(name.size()-3, 3);
        std::string strFindName = "([0-6]" + subName1 + "\\w{6}" + subName2 + "\\w)";
        std::regex txt_regex(strFindName);
        std::smatch base_match;
        for(auto itr = m_mapNameChannel.begin(); itr != m_mapNameChannel.end(); itr++){
            if(std::regex_match(itr->first, base_match, txt_regex)) {
                if(base_match.size() == 2)
                {
                    vecChannel.push_back(itr->second);
                    break;
                }
            }
        }
    }
    return vecChannel.size();
}


int ChannelManagerBase::findChannelArrayFromNamesRegex(std::vector<std::string>& vecName, TArray<Channel*>& taChannel)
{
    taChannel.setSize(m_mapNameChannel.size());
    int index = 0;
    for(auto & name : vecName)
    {
        std::string subName1 = name.substr(0, 5);
        std::string subName2 = name.substr(name.size()-3, 3);
        std::string strFindName = "([0-6]" + subName1 + "\\w{6}" + subName2 + "\\w)";
        std::regex txt_regex(strFindName);
        std::smatch base_match;
        for(auto itr = m_mapNameChannel.begin(); itr != m_mapNameChannel.end(); itr++){
            if(std::regex_match(itr->first, base_match, txt_regex)) {
                if(base_match.size() == 2)
                {
                    taChannel[index] = itr->second;
                    index++;
                    break;
                }
            }
        }
    }
    taChannel.resize(index);
    return taChannel.getCount();
}
```
---



