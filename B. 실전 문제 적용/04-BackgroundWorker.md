## 📘 C++ BackgroundWork 구현

### 📌 1. C# BackgroundWorker 핵심 기능
- 비동기 실행: RunWorkerAsync()로 작업 시작
- 진행률 보고: ReportProgress() → ProgressChanged 이벤트 발생
- 작업 완료 알림: RunWorkerCompleted 이벤트 발생
- 취소 지원: CancellationPending 플래그로 중단

### 📌 2. C++에서 구현 전략
- C++에서는 std::thread + std::function + std::mutex + std::condition_variable 등을 조합해 구현.
- 이벤트 시스템은 콜백(callback) 또는 observer 패턴으로 대체합니다.

### 📌 3. 예제 구현 (C++17 기준)
```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>

class BackgroundWorker {
public:
    using DoWorkHandler = std::function<void(std::atomic<bool>&, std::function<void(int)>)>;
    using ProgressHandler = std::function<void(int)>;
    using CompletedHandler = std::function<void()>;

    void setDoWork(DoWorkHandler handler) { doWorkHandler = handler; }
    void setProgressChanged(ProgressHandler handler) { progressHandler = handler; }
    void setCompleted(CompletedHandler handler) { completedHandler = handler; }

    void runWorkerAsync() {
        cancelRequested = false;
        workerThread = std::thread([this]() {
            if (doWorkHandler) {
                doWorkHandler(cancelRequested, [this](int progress) {
                    if (progressHandler) progressHandler(progress);
                });
            }
            if (completedHandler) completedHandler();
        });
    }

    void cancelAsync() { cancelRequested = true; }

    void wait() {
        if (workerThread.joinable()) workerThread.join();
    }

private:
    std::thread workerThread;
    std::atomic<bool> cancelRequested{false};
    DoWorkHandler doWorkHandler;
    ProgressHandler progressHandler;
    CompletedHandler completedHandler;
};

// -------------------- 사용 예시 --------------------
int main() {
    BackgroundWorker worker;

    worker.setDoWork([](std::atomic<bool>& cancel, std::function<void(int)> reportProgress) {
        for (int i = 0; i <= 100; i += 10) {
            if (cancel) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            reportProgress(i);
        }
    });

    worker.setProgressChanged([](int progress) {
        std::cout << "Progress: " << progress << "%\n";
    });

    worker.setCompleted([]() {
        std::cout << "Work Completed!\n";
    });

    worker.runWorkerAsync();

    // 1초 후 취소 시도
    std::this_thread::sleep_for(std::chrono::seconds(1));
    worker.cancelAsync();

    worker.wait();
}
```
#### 🔹 결과 출력
```
Progress: 0%
Progress: 10%
Progress: 20%
Progress: 30%
Progress: 40%
Work Completed!

```

### 📌 4. 구현 포인트
- std::atomic<bool> → C#의 CancellationPending 역할
- 람다 + std::function → C# 이벤트 대체
- reportProgress 콜백 → ReportProgress() 역할
- setCompleted → RunWorkerCompleted 역할
- wait() → C#의 Join()과 유사


## 📘 BackgroundWork ThreadPool 구현

### 📌 1. 설계 개념
- 스레드 풀: 여러 작업을 큐에 넣고, 제한된 수의 스레드가 순차적으로 처리
- std::promise / std::future: 작업 결과를 비동기적으로 반환
- 진행률 보고: 콜백 함수로 처리
- 취소 지원: std::atomic<bool> 플래그 사용

### 📌 2. 간단한 스레드 풀 + BackgroundWorker 스타일 구현
```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <future>
#include <functional>
#include <atomic>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// -------------------- BackgroundWorker 스타일 --------------------
struct BackgroundWorker {
    ThreadPool& pool;
    std::atomic<bool> cancelFlag{false};

    template<typename Func, typename ProgressFunc>
    auto runAsync(Func work, ProgressFunc progress) {
        return pool.enqueue([this, work, progress]() {
            return work(cancelFlag, progress);
        });
    }

    void cancel() { cancelFlag = true; }
};

// -------------------- 사용 예시 --------------------
int main() {
    ThreadPool pool(4);
    BackgroundWorker worker{pool};

    auto futureResult = worker.runAsync(
        [](std::atomic<bool>& cancel, std::function<void(int)> reportProgress) -> int {
            int sum = 0;
            for (int i = 0; i <= 100; i += 10) {
                if (cancel) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                sum += i;
                reportProgress(i);
            }
            return sum;
        },
        [](int progress) {
            std::cout << "Progress: " << progress << "%\n";
        }
    );

    // 1초 후 취소
    std::this_thread::sleep_for(std::chrono::seconds(1));
    worker.cancel();

    std::cout << "Result: " << futureResult.get() << "\n";
}
```

#### 🔹 결과 출력
```
Progress: 0%
Progress: 10%
Progress: 20%
Progress: 30%
Result: Progress: 40%
100
```

### 📌 3. 특징
- 스레드 풀 기반 → 여러 작업을 동시에 처리 가능
- std::future → 작업 결과를 비동기적으로 받아옴
- std::promise → 내부적으로 packaged_task가 사용되어 결과 전달
- 진행률 보고 → 콜백 함수로 즉시 반영
- 취소 지원 → std::atomic<bool>로 안전하게 중단

### 📌 4. C# BackgroundWorker와 비교
| 기능             | C# BackgroundWorker                                   | C++ (ThreadPool + future/promise)                  |
|------------------|-------------------------------------------------------|-----------------------------------------------------|
| 작업 시작         | `RunWorkerAsync()` 호출                               | `pool.enqueue()` 또는 `worker.runAsync()` 호출      |
| 작업 실행 방식    | 내부적으로 스레드 생성 및 관리                        | 스레드 풀에서 유휴 스레드 할당                      |
| 결과 반환         | `RunWorkerCompleted` 이벤트에서 `e.Result` 사용       | `std::future<T>`의 `.get()`으로 결과 획득           |
| 진행률 보고       | `ReportProgress(int)` → `ProgressChanged` 이벤트 발생 | 진행률 콜백 함수 호출 (`progress(int)`)             |
| 취소 요청         | `CancelAsync()` → `CancellationPending` 플래그 확인   | `cancelFlag`(`std::atomic<bool>`) 확인              |
| 예외 처리         | `RunWorkerCompleted`의 `e.Error`로 전달               | `future.get()` 호출 시 예외 전파                    |
| 동기 대기         | `RunWorkerCompleted` 이벤트까지 대기                  | `future.wait()` 또는 `.get()`으로 대기              |
| 스레드 재사용     | 매 실행 시 새 스레드 생성                             | 스레드 풀에서 재사용 → 성능 효율 ↑                  |
| 이벤트 기반       | `DoWork`, `ProgressChanged`, `RunWorkerCompleted`     | 콜백 함수 또는 람다로 직접 지정                      |
| 언어 지원         | .NET(C#, VB 등)                                       | C++17 이상 (표준 라이브러리 기반)                   |

- 이 구조를 쓰면 C# BackgroundWorker의 핵심 기능을 C++에서 스레드 풀 + future/promise로 거의 동일하게 재현할 수 있습니다.


### 📌 플랫폼 독립적으로 쓰는 방법
- 핵심은 UI 스레드에서만 UI를 건드린다는 원칙을 지키면서,  
OS 종속 API 대신 표준 C++과 프레임워크 중립적인 이벤트 전달 방식을 쓰는 겁니다.

#### 🔹 1. 공유 상태 + 폴링(Timer) 방식
- 작업 스레드: 진행률을 std::atomic이나 뮤텍스로 보호된 변수에 저장
- UI 스레드: 주기적으로(타이머, 메인 루프) 그 값을 읽어 UI 갱신
- 장점: OS API 의존 없음, 어디서나 동작
- 단점: 폴링 주기만큼 반응이 늦어질 수 있음

```cpp
std::atomic<int> progress{0};

void worker() {
    for (int i = 0; i <= 100; ++i) {
        progress.store(i, std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
// UI 루프 안에서
if (progressChangedSinceLastCheck()) {
    updateUI(progress.load());
}

```

#### 🔹 2. 스레드 안전 큐 + 메인 루프 처리
- 작업 스레드: 이벤트(예: 진행률 값)를 스레드 안전 큐에 push
- UI 스레드: 메인 루프에서 큐를 비우면서 UI 갱신
- 장점: 변화가 있을 때만 처리 → 불필요한 호출 없음
- 단점: UI 루프에서 주기적으로 큐를 확인해야 함
```cpp
#include <queue>
#include <mutex>

std::queue<int> progressQueue;
std::mutex queueMutex;

void reportProgress(int value) {
    std::lock_guard<std::mutex> lock(queueMutex);
    progressQueue.push(value);
}

void processUIEvents() {
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!progressQueue.empty()) {
        int value = progressQueue.front();
        progressQueue.pop();
        updateUI(value);
    }
}
```

### 📌 전체 코드 예제
```cpp
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>
#include <iostream>
#include <optional>

// ===== 이벤트 정의 =====
enum class EventType {
    Progress,
    Completed
};

struct Event {
    EventType type;
    int value; // Progress일 때만 사용
};

// ===== 스레드 안전 큐 =====
class EventQueue {
public:
    void push(const Event& ev) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(ev);
    }

    bool try_pop(Event& ev) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) return false;
        ev = queue_.front();
        queue_.pop();
        return true;
    }

private:
    std::queue<Event> queue_;
    std::mutex mtx_;
};

// ===== BackgroundWorker =====
class BackgroundWorker {
public:
    using ProgressHandler   = std::function<void(int)>;
    using CompletionHandler = std::function<void()>;

    BackgroundWorker() : running_(false) {}
    ~BackgroundWorker() { cancel(); wait(); }

    void setProgressHandler(ProgressHandler handler) {
        progressHandler_ = std::move(handler);
    }

    void setCompletionHandler(CompletionHandler handler) {
        completionHandler_ = std::move(handler);
    }

    void start() {
        if (running_) return;
        running_ = true;
        workerThread_ = std::thread([this]() {
            for (int i = 0; i <= 100 && running_; i += 10) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                eventQueue_.push({ EventType::Progress, i });
            }
            eventQueue_.push({ EventType::Completed, 0 });
        });
    }

    void pollEvents() {
        Event ev;
        while (eventQueue_.try_pop(ev)) {
            if (ev.type == EventType::Progress && progressHandler_) {
                progressHandler_(ev.value);
            }
            else if (ev.type == EventType::Completed) {
                if (completionHandler_) completionHandler_();
                running_ = false; // 종료 상태 표시
            }
        }
    }

    bool isRunning() const { return running_; }

    void wait() {
        if (workerThread_.joinable())
            workerThread_.join();
    }

    void cancel() {
        running_ = false;
    }

private:
    std::atomic<bool> running_;
    std::thread workerThread_;
    EventQueue eventQueue_;
    ProgressHandler progressHandler_;
    CompletionHandler completionHandler_;
};
int main() {
    BackgroundWorker worker;

    // 진행 상황 콜백
    worker.setProgressHandler([](int progress) {
        std::cout << "Progress: " << progress << "%\n";
    });

    // 완료 콜백
    worker.setCompletionHandler([]() {
        std::cout << "Completed!\n";
    });

    // 작업 시작
    worker.start();

    // 메인 루프: 작업이 끝날 때까지 이벤트 처리
    while (worker.isRunning()) {
        worker.pollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // 혹시 남아있는 이벤트 처리
    worker.pollEvents();

    // 스레드 종료 대기
    worker.wait();

    std::cout << "Main thread exiting.\n";
    return 0;
}
```

#### 🔹 특징
- OS 독립적: Windows, Linux, macOS 어디서나 빌드 가능
- UI 스레드 안전성 보장: UI 스레드에서만 핸들러 실행
- 이벤트 기반: 진행률이 변할 때만 이벤트 발생
- C# BackgroundWorker 스타일: onProgressChanged, onCompleted 제공

#### 🔹 이 구조를 쓰면,
- Windows에서는 Win32/MFC,
- macOS에서는 Cocoa,
- Linux에서는 GTK/Qt
- 어디든 메인 루프에서 pollEvents()만 호출하면 됩니다.


### 📌 간단하고 OS 독립적인 패턴
- 사실 우리가 앞서 만든 BackgroundWorker 스타일 + 이벤트 큐 구조가  
이런 “파일 읽기 / 웹 요청 → 완료 시 UI 피드백”에 딱 맞습니다.
- 흐름
    - 작업 스레드에서 파일 읽기나 HTTP 요청 수행
    - 완료 시 스레드 안전 큐에 “완료 이벤트” push
    - UI 스레드에서 타이머나 메인 루프에서 pollEvents() 호출
    - 큐에서 이벤트 꺼내서 UI 업데이트

#### 🔹 예시: 파일 읽기
```cpp
worker.onCompleted([]() {
    // UI 스레드에서 실행됨
    showMessage("파일 읽기 완료!");
});

worker.start([]() {
    std::ifstream file("data.txt");
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    // 필요하면 progress 이벤트도 중간중간 report
});
```


#### 🔹 예시: 웹 요청 (libcurl)
```cpp
worker.onCompleted([]() {
    updateStatusLabel("데이터 수신 완료");
});

worker.start([]() {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/data");
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
});
```


### 📌 장점
- OS 독립: Windows, Linux, macOS 어디서나 동일하게 동작
- UI 스레드 안전성: UI 업데이트는 항상 메인 스레드에서만
- 간단한 코드 흐름: async/future처럼 복잡한 동기화 코드 불필요
- 확장성: 진행률, 취소, 예외 처리 쉽게 추가 가능



### 📌 Qt 연동 예시
```cpp
#include <QApplication>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <iostream>

// BackgroundWorker 헤더 포함
// #include "BackgroundWorker.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        auto *central = new QWidget(this);
        auto *layout = new QVBoxLayout(central);

        progressBar_ = new QProgressBar;
        progressBar_->setRange(0, 100);
        layout->addWidget(progressBar_);

        auto *startBtn = new QPushButton("작업 시작");
        layout->addWidget(startBtn);

        setCentralWidget(central);

        // Worker 콜백 설정
        worker_.setProgressHandler([this](int value) {
            // UI 스레드에서 실행되므로 안전
            progressBar_->setValue(value);
        });

        worker_.setCompletionHandler([this]() {
            progressBar_->setValue(100);
            std::cout << "작업 완료!" << std::endl;
        });

        // 버튼 클릭 시 작업 시작
        connect(startBtn, &QPushButton::clicked, this, [this]() {
            worker_.start();
        });

        // QTimer로 pollEvents 주기 호출
        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() {
            worker_.pollEvents();
        });
        timer->start(50); // 50ms마다 이벤트 처리
    }

private:
    QProgressBar *progressBar_;
    BackgroundWorker worker_;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
```


#### 🔹 동작 방식
- 버튼 클릭 → worker_.start()로 백그라운드 스레드 시작
- 백그라운드 작업 → 진행률/완료 이벤트를 큐에 넣음
- QTimer → 50ms마다 pollEvents() 호출
- UI 업데이트 → 진행률 바와 콘솔 출력


### 📌 MFC용 BackgroundWorker 예시
```cpp
// BackgroundWorkerMFC.h
#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <windows.h>

enum class WorkerEventType {
    Progress,
    Completed
};

struct WorkerEvent {
    WorkerEventType type;
    int value; // Progress일 때만 사용
};

// 사용자 정의 메시지
#define WM_WORKER_PROGRESS  (WM_USER + 1)
#define WM_WORKER_COMPLETED (WM_USER + 2)

class BackgroundWorkerMFC {
public:
    BackgroundWorkerMFC(HWND hWndTarget)
        : hWndTarget_(hWndTarget), running_(false) {}

    ~BackgroundWorkerMFC() {
        cancel();
        wait();
    }

    void start() {
        if (running_) return;
        running_ = true;

        workerThread_ = std::thread([this]() {
            for (int i = 0; i <= 100 && running_; i += 10) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                PostMessage(hWndTarget_, WM_WORKER_PROGRESS, (WPARAM)i, 0);
            }
            PostMessage(hWndTarget_, WM_WORKER_COMPLETED, 0, 0);
        });
    }

    void cancel() {
        running_ = false;
    }

    bool isRunning() const { return running_; }

    void wait() {
        if (workerThread_.joinable())
            workerThread_.join();
    }

private:
    HWND hWndTarget_;
    std::atomic<bool> running_;
    std::thread workerThread_;
};

```
#### 🔹 MFC 대화상자/뷰에서 사용 예시
```cpp
// MyDialog.h
#pragma once
#include "BackgroundWorkerMFC.h"

class CMyDialog : public CDialogEx {
public:
    CMyDialog(CWnd* pParent = nullptr)
        : CDialogEx(IDD_MY_DIALOG, pParent),
          worker_(m_hWnd) {}

protected:
    virtual BOOL OnInitDialog() {
        CDialogEx::OnInitDialog();
        return TRUE;
    }

    afx_msg void OnBnClickedStart() {
        worker_ = BackgroundWorkerMFC(m_hWnd); // 새로 생성
        worker_.start();
    }

    afx_msg LRESULT OnWorkerProgress(WPARAM wParam, LPARAM lParam) {
        int progress = (int)wParam;
        m_ProgressCtrl.SetPos(progress);
        return 0;
    }

    afx_msg LRESULT OnWorkerCompleted(WPARAM wParam, LPARAM lParam) {
        AfxMessageBox(_T("작업 완료!"));
        return 0;
    }

    DECLARE_MESSAGE_MAP()

private:
    BackgroundWorkerMFC worker_;
    CProgressCtrl m_ProgressCtrl;
};
```

#### 🔹 메시지 맵 연결
```cpp
BEGIN_MESSAGE_MAP(CMyDialog, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_START, &CMyDialog::OnBnClickedStart)
    ON_MESSAGE(WM_WORKER_PROGRESS, &CMyDialog::OnWorkerProgress)
    ON_MESSAGE(WM_WORKER_COMPLETED, &CMyDialog::OnWorkerCompleted)
END_MESSAGE_MAP()
```

### 📌 동작 흐름
- start() 호출 → 백그라운드 스레드 시작
- 진행률 발생 시 PostMessage(WM_WORKER_PROGRESS, progress, 0) 호출
- UI 스레드에서 ON_MESSAGE 매핑된 핸들러 실행 → 안전하게 UI 업데이트
- 완료 시 WM_WORKER_COMPLETED 메시지 전송 → UI에서 완료 처리

### 📌 이렇게 하면 MFC에서도 C# BackgroundWorker처럼
- 진행률 보고
- 완료 알림
- UI 스레드 안전성 보장

---


