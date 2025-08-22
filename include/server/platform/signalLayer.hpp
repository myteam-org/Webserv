#pragma once

#include <signal.h>

namespace platform {

enum SignalAction {
    kSigNone = 0,
    kSigGracefulStop = 1,
    kSigReload = 2,
    kSigChild = 3
};

class SignalLayer {
   public:
    SignalLayer();
    ~SignalLayer();

    // 初期化成功 true / 失敗 false。  errが非NULLなら理由を格納 
    bool init(std::string* err);

    // 読み取り側fd(eploll/kqueue に登録するのはこれ)
    int fd() const;
    
    // EPOLLINなどの通知を受けてから呼ぶ。readの戻り値だげで判定しerrnoを見ない
    bool drainOnce(SignalAction* act) const;

    // パイプ漏れ時やシグナル嵐に備えた保険のフラグ消費(I/O無し)
    bool takePending(SignalAction* act);
 
    void shutdown();


   private:
    SignalLayer(const SignalLayer&);
    SignalLayer& operator=(const SignalLayer);

    static void onSignal(int signo); // signal()用ハンドラ
    
    static volatile sig_atomic_t sWriteFd_; // self-pipe 書き込み側
    static volatile sig_atomic_t sFlags_;

    int rfd_;
    int wfd_;
};
} // namespace platform
