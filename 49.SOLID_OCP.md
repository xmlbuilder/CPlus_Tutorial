# 📘 SOLID 원칙 정리 - OCP 중심

## 🔹 SOLID 원칙 개요

SOLID는 객체지향 설계의 5대 원칙을 의미합니다.

-   **SRP (Single Responsibility Principle)** - 단일 책임 원칙\
-   **OCP (Open-Closed Principle)** - 개방 폐쇄 원칙\
-   **LSP (Liskov Substitution Principle)** - 리스코프 치환 원칙\
-   **ISP (Interface Segregation Principle)** - 인터페이스 분리 원칙\
-   **DIP (Dependency Inversion Principle)** - 의존 관계 역전 원칙

------------------------------------------------------------------------

## 🔹 SRP (Single Responsibility Principle)

> 클래스와 메서드는 **하나의 역할만** 하도록 설계해야 한다.
> Java를 기반으로 원리를 설명한다.

### ❌ 잘못된 예시 (SRP 위반)

``` java
public class LoggingService {
  private DataSource loggingDB = new MySQLDataSource();
  
  // 로그를 출력하고 저장하는 비즈니스 로직
}
```

-   `LoggingService`가 두 가지 책임을 가짐
    1)  `loggingDB` 객체 생성\
    2)  로그 출력 및 저장

이는 SRP를 위배한다.

### ✅ 개선된 예시 (의존성 주입 적용)

``` java
public class LoggingService {
  @Autowired
  private DataSource loggingDB;
  
  // 로그를 출력하고 저장하는 비즈니스 로직
}
```

-   객체 생성 책임을 외부로 분리하여 **단일 책임 원칙**을 지킬 수 있음.

------------------------------------------------------------------------

## 🔹 OCP (Open-Closed Principle)

> 소프트웨어 엔티티(클래스, 모듈, 함수 등)는 **확장에는 열려(Open)**
> 있어야 하고, **변경에는 닫혀(Closed)** 있어야 한다.

### ❌ 잘못된 예시 (SRP & OCP 동시 위반)

``` java
public class LoggingService {
    private DataSource loggingDB = new MySQLDataSource();
    
    // 로그를 출력하고 저장하는 비즈니스 로직
}
```

-   새로운 데이터베이스(MongoDB 등)를 추가할 경우 `LoggingService`
    코드를 수정해야 한다.\
-   즉, **변경에도 열려 있는 상태** → OCP 위반.

### ✅ 개선된 예시 (OCP 준수)

``` java
public interface DataSource {
    void saveLog(String message);
}

public class MySQLDataSource implements DataSource {
    public void saveLog(String message) {
        // MySQL 저장 로직
    }
}

public class MongoDBDataSource implements DataSource {
    public void saveLog(String message) {
        // MongoDB 저장 로직
    }
}

public class LoggingService {
    private DataSource loggingDB;

    public LoggingService(DataSource dataSource) {
        this.loggingDB = dataSource;
    }

    public void log(String message) {
        loggingDB.saveLog(message);
    }
}
```

-   새로운 데이터베이스를 추가할 경우, **새로운 `DataSource` 구현체만
    작성**하면 된다.\
-   기존 `LoggingService` 코드를 수정할 필요 없음 → **변경에는 닫혀
    있고, 확장에는 열려 있음**.

------------------------------------------------------------------------

## 📝 결론

-   **SRP**: 클래스는 하나의 책임만 가져야 한다.\
-   **OCP**: 코드는 **확장 가능**하되, **수정은 최소화**해야 한다.\
-   개선된 설계에서는 **인터페이스와 의존성 주입(DI)**를 통해 SOLID
    원칙을 자연스럽게 지킬 수 있다.
