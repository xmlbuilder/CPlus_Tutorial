# 🪟 Qt + `std::async` + `QBasicTimer` — 진행 표시 UI 예제 (정리 & 개선)

Qt GUI에서 **백그라운드 작업 진행률**을 표시하는 예제를 정리했습니다.
GUI 스레드에서만 위젯 접근(예: `progressBar->setValue`)하는 것은 **중요한 **일입니다 — 타이머로 메인 스레드에서만 UI를 만지도록 되어 있어 안전합니다.

---

## ✅ 최소 변경 / 안전한 동기화

- `m_nProgress` → `std::atomic<int>`
- `**`std::launch::async`** 로 강제(지연 실행 방지)
- 완료 시점의 UI 변경은 **메인 스레드로 우회** (`QMetaObject::invokeMethod` with `Qt::QueuedConnection`)
- 소멸자에서 **`f.wait()`** 로 안전 종료

### `mainwindow.h`

```cpp
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBasicTimer>
#include <future>
#include <atomic>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void timerEvent(QTimerEvent* event) override;

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui = nullptr;

    std::future<int> f;
    std::atomic<int> m_progress{0};   // <-- thread-safe
    QBasicTimer      m_timer;
};

#endif // MAINWINDOW_H
```

### `mainwindow.cpp`

```cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <thread>
#include <future>
#include <QObject>

// 백그라운드 작업: 매 초 콜백 호출, 마지막에 종료 신호
static int func_cb(std::function<int(int, int)> finished)
{
    for (int i = 0; i < 10; ++i) {
        std::cout << "do something..." << std::endl;
        finished(0, i); // 진행
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    finished(1, 100);   // 완료
    return 1;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    // 백그라운드가 살아있다면 종료까지 대기(예외 전파 불필요하면 wait, 필요하면 try { get(); } catch ... )
    if (f.valid()) f.wait();
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        const int v = m_progress.load(std::memory_order_relaxed);
        ui->progressBar->setValue(v * 10); // 0..9 -> 0..90 (마지막은 finished에서 100으로 설정)
    } else {
        QWidget::timerEvent(event);
    }
}

void MainWindow::on_pushButton_clicked()
{
    if (f.valid()) return; // 이미 실행 중이면 무시

    // 주기적으로 UI 갱신
    m_timer.start(200, this);

    // 백그라운드 실행을 확실히 강제
    f = std::async(std::launch::async, &func_cb,
                   // 콜백: 작업 스레드에서 호출됨 (UI 직접 접근 금지)
                   [this](int type, int value) -> int {
                       if (type == 0) {
                           // 진행률 업데이트 (스레드-세이프)
                           m_progress.store(value, std::memory_order_relaxed);
                       } else if (type == 1) {
                           // 완료: 메인 스레드에서 타이머 정지 및 최종 표시
                           QMetaObject::invokeMethod(this, [this] {
                               m_timer.stop();
                               ui->progressBar->setValue(100);
                           }, Qt::QueuedConnection);
                       }
                       return 0;
                   });
}
```

---

## 🧪 동작 흐름
1. 버튼 클릭 → `std::async`로 `func_cb`가 **작업 스레드에서** 시작
2. 매 초 콜백 호출 → `m_progress`를 원자적으로 업데이트
3. `QBasicTimer`가 **메인 스레드에서** `timerEvent`로 주기적 UI 갱신
4. 작업 종료 시 콜백(type==1) → `invokeMethod`로 메인 스레드에서 타이머 정지/완료 표시
5. 소멸자에서 `f.wait()`로 안전 종료

---

## 💡 대안: Qt-native (추천)
Qt만으로도 같은 구조를 더 자연스럽게 만들 수 있습니다.
- `QtConcurrent::run()` + `QFutureWatcher`로 진행/완료 시그널을 GUI 스레드로 자동 전달
- 또는 **`QThread` + QObject worker + signal/slot`** 패턴

하지만 위 수정판처럼 `std::async` + `QBasicTimer` 조합도 **충분히 안전**하게 동작합니다.

---

## 🛠 빌드 팁
- C++17 이상 권장 (`std::atomic`, `std::async` 사용)
- `std::launch::async` 사용 시, future 파괴 전에 작업이 끝나도록 **`wait()`/`get()`** 호출을 잊지 마세요.

---
