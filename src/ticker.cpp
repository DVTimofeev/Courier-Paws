#include "ticker.h"

Ticker::Ticker(Strand& strand, std::chrono::milliseconds period, Handler handler) 
    : strand_{strand}
    , period_{period}
    , handler_{handler}
    , timer_{strand_}
    {}

void Ticker::Start() {
        last_tick_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
        ScheduleTick();
    }

void Ticker::ScheduleTick() {
    timer_.expires_after(period_);
    timer_.async_wait(net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec){
        self->OnTick(ec);
    }));
}

void Ticker::OnTick(sys::error_code ec) {
    auto current_tick = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    handler_(current_tick - last_tick_);
    last_tick_ = current_tick;
    ScheduleTick();
}