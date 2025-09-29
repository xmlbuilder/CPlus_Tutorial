#  vcpkg로 테스트 통합
- vcpkg로 gtest 설치 후 MSBuild 프로젝트에 연결 (CMake 없이)
- 기존 .vcxproj를 그대로 쓰면서 의존성만 가져오고 싶을 때 좋음.

## vcpkg 설치 & 통합
```
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg integrate install   # VS 전체 통합
```

## (x64 예시) gtest 설치
```
.\vcpkg\vcpkg install gtest:x64-windows
```
- VS에서 테스트 프로젝트(.vcxproj) 속성에 아래 확인
- vcpkg 통합이 켜져 있으면 포함 경로/라이브러리 경로가 자동 설정된다.

## 자동이 안 되면 수동으로:
```
C/C++ → Additional Include Directories
$(VCPKG_ROOT)\installed\x64-windows\include

Linker → Additional Library Directories
$(VCPKG_ROOT)\installed\x64-windows\lib (Release),
$(VCPKG_ROOT)\installed\x64-windows\debug\lib (Debug)
```
- Linker → Additional Dependencies
    - gtest.lib; (Debug도 동일 이름, 경로만 debug로 잡힘)

## 런타임 라이브러리 일치

- 모든 프로젝트가 같은 CRT를 쓰도록
    -C/C++ → Code Generation → Runtime Library 를 /MD(Release), /MDd(Debug)로 통일.
        (혼재하면 링크 에러/경고 발생)

- 테스트 실행은 Test Explorer에서.
-  gtest.lib를 링크

## 최소 예제 (테스트 프로젝트의 .cpp)
```cpp
#include <gtest/gtest.h>

// 테스트할 코드
int add(int a, int b) { return a + b; }

// 테스트
TEST(Math, Add) {
    EXPECT_EQ(add(2, 3), 5);
    EXPECT_NE(add(-1, 1), 1);
}

// gtest_main을 링크하지 않는다면 main 필요

 int main(int argc, char** argv) {
     ::testing::InitGoogleTest(&argc, argv);
     return RUN_ALL_TESTS();
}
 ```

## Test Explorer가 테스트를 못 찾을 때 체크리스트
- VS에 C++ 테스트 어댑터가 활성화되어 있어야 함(최신 VS는 기본 포함).
- Debug/Release 둘 다 빌드해 보고, 테스트 대상 구성(플랫폼/구성)이 올바른지 확인.
- 실행 파일이 Windows 앱 서브시스템(/SUBSYSTEM:WINDOWS) 가 아닌 콘솔(/SUBSYSTEM:CONSOLE) 인지.
- 프로젝트가 x64와 x86 혼용되어 있지 않은지(의존성 일치).
- vcpkg 사용 시 빌드 구성과 triplet(x64-windows 등) 가 맞는지.

## VS에서 디버깅 팁
- Test Explorer에서 테스트 항목 우클릭 → Debug.
- 브레이크포인트를 테스트/프로덕션 코드에 걸고 그대로 추적.
- 실패 시 Output 창에서 gtest가 출력한 메시지로 빠르게 원인 파악.
