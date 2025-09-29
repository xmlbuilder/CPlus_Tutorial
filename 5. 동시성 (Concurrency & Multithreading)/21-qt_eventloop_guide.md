# 🔄 Qt Event Loop 정리 — `QEventLoop`, `processEvents()`, Busy Loop 대안

Qt 애플리케이션은 **이벤트 루프**(Event Loop)가 신호/이벤트/타이머/네트워크 I/O를 처리하면서 동작합니다.  
이 문서는 *Busy Loop*를 피하고, `QEventLoop`와 `QCoreApplication::processEvents()`를 안전하게 사용하는 방법을 예제로 정리합니다.

---

## ✅ 핵심 요약
- **Busy Loop(while + sleep)** 는 CPU를 낭비하고, **신호·이벤트가 처리되지 않아** UI가 멈출 수 있습니다.
- **`QEventLoop`** 는 현재 스레드에서 이벤트를 처리하면서 **특정 시점(시그널)** 까지 **블록**할 때 유용합니다.
- **오래 걸리는 작업**은 **다른 스레드**(예: `QThread`, `QtConcurrent`)로 옮기는 것이 정석입니다.
- 부득이하게 현재 스레드에서 진행해야 한다면, **주기적으로 `QCoreApplication::processEvents()`** 로 이벤트를 처리하세요(남용은 금물).

---

## ❌ Busy Loop (지양)
```cpp
// Bad: Main Loop (CPU 낭비 + 이벤트 미처리)
while (antThreadRunning) {
  QThread::msleep(150);
}
```
- 이벤트 루프가 돌지 않으므로 UI/타이머/네트워크 콜백이 멈출 수 있습니다.

---

## ✅ `QEventLoop`로 신호를 기다리기 (권장)

### 1) 스레드 종료를 기다리기
```cpp
QEventLoop loop;
QObject::connect(thread, SIGNAL(antThreadRunningFinish()), &loop, SLOT(quit()));
loop.exec();  // quit() 시그널이 오면 반환
```

### 2) 타임아웃과 함께 기다리기
```cpp
QEventLoop loop;
QTimer timer;
QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
int runTimeMs = seconds * 1000;
timer.start(runTimeMs);
loop.exec();  // 타임아웃되면 quit() → exec() 반환
```

> ✅ `QEventLoop`는 **이벤트를 처리**하면서 대기하므로 Busy Loop보다 효율적이며,  
> 타이머·신호·네트워크 이벤트가 정상적으로 동작합니다.

---

## 🌐 비동기 네트워크 응답 기다리기 (`QNetworkReply`)
`QNetworkReply`는 블록킹 API가 없습니다. 임시 이벤트 루프를 사용해 **응답 완료까지** 기다립니다.

```cpp
QNetworkAccessManager qnam;
QNetworkReply *reply = qnam.get(QNetworkRequest(QUrl("https://example.com")));

QEventLoop loop;
QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
loop.exec();

// finished() 이후 결과 처리
QByteArray body = reply->readAll();
reply->deleteLater();
```

> ⚠️ 과도한 **중첩 이벤트 루프(re-entrancy)** 는 로직을 복잡하게 만듭니다.  
> 가능하면 **순수 비동기**(시그널 기반 처리)나 **워커 스레드**를 우선 고려하세요.

---

## 🧰 오래 걸리는 작업 + 이벤트 유지 (차선책)

### A) 현재 스레드에서 이벤트 펌프 돌리기
```cpp
void longTask() {
  for (int i = 0; i < 1000; ++i) {
    // ... heavy work ...
    if (i % 10 == 0) {
      QCoreApplication::processEvents(); // 이벤트 처리 후 계속
    }
  }
}
```
- **장점**: 간단, 현재 스레드(예: GUI)에서 진행 가능  
- **단점**: **재진입(re-entrancy)** 위험, 이벤트 핸들러가 다시 `longTask`를 호출하는 등 설계 복잡해질 수 있음

### B) 임시 이벤트 루프로 재진입
```cpp
QEventLoop loop;
QTimer::singleShot(0, &loop, SLOT(quit())); // 다음 틱에 빠져나오도록
loop.exec(); // 그 사이 이벤트 처리
```
- 반복적으로 `exec()` 를 호출하여 **이벤트 빈틈을 메울 수는 있지만**, 가급적 지양

---

## 🧵 정석: 워커 스레드로 옮기기

### 1) `QThread` + Worker (요약)
```cpp
// Worker는 QObject로 만들고, moveToThread로 스레드로 이동
auto* thread = new QThread;
auto* worker = new Worker;    // QObject 파생, doWork() 슬롯 보유
worker->moveToThread(thread);
QObject::connect(thread,  &QThread::started, worker,  &Worker::doWork);
QObject::connect(worker,  &Worker::finished, thread,  &QThread::quit);
QObject::connect(thread,  &QThread::finished, worker, &QObject::deleteLater);
QObject::connect(thread,  &QThread::finished, thread, &QObject::deleteLater);
thread->start();
```

### 2) `QtConcurrent::run` + `QFutureWatcher`
```cpp
QFutureWatcher<void> watcher;
QObject::connect(&watcher, &QFutureWatcher<void>::finished, []{
    // 완료 시 GUI 업데이트
});
watcher.setFuture(QtConcurrent::run([]{
    // 긴 작업
}));
```

---

## 🧪 현대식 시그널/슬롯 문법 (권장)
```cpp
QEventLoop loop;
QTimer timer;

QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
timer.start(1000);
loop.exec();
```

> ✅ 새 문법은 **컴파일러 타입 체크**가 가능하고, 리팩토링에 안전합니다.

---

## 📎 체크리스트
- [x] Busy Loop 대신 `QEventLoop`를 사용해 **이벤트를 처리하면서 대기**한다.
- [x] **오래 걸리는 작업은 별도 스레드**로 옮긴다(`QThread`, `QtConcurrent`).
- [x] 불가피하게 현재 스레드에서 작업한다면, **주기적으로 `processEvents()`** 를 호출한다(남용 금지).
- [x] 중첩 이벤트 루프는 **최소화**한다.
- [x] 네트워크/타이머/신호를 기다릴 때는 **임시 `QEventLoop` + `quit()`** 패턴을 활용한다.

---

